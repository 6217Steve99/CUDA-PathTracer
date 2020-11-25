#pragma once
#include "common.h"
#include "ray.h"
#include "Sampling.h"
#include <thrust/random.h>

class Homogeneous {
public:
	float3 sigmaA, sigmaS, sigmaT;

public:
	__device__ float3 Tr(const Ray& ray, thrust::uniform_real_distribution<float>& uniform, thrust::default_random_engine& rng) const {
		float3 c = sigmaT * (-ray.tmax);
		return Exp(c);
	}

	__device__ float3 Sample(const Ray& ray, thrust::uniform_real_distribution<float>& uniform, thrust::default_random_engine& rng, float& t, bool& sampled) const {
		int channel = uniform(rng) * 3;
		channel = channel == 3 ? 2 : channel;
		float dist = -std::log(1 - uniform(rng)) / (&sigmaT.x)[channel];
		dist = dist < ray.tmax ? dist : ray.tmax;
		sampled = dist < ray.tmax;
		if (sampled)
			t = dist;

		// Compute the transmittance and sampling density
		float3 Tr = Exp(sigmaT * -dist);

		// Return weighting factor for scattering from homogeneous medium
		float3 density = sampled ? (sigmaT * Tr) : Tr;
		float pdf = 0;
		for (int i = 0; i < 3; ++i) pdf += (&density.x)[i];
		pdf /= 3.f;
		if (pdf == 0) pdf = 1;

		return sampled ? (Tr * sigmaS / pdf) : (Tr / pdf);
	}
};

class Heterogeneous {
public:
	float3 sigmaA, sigmaS, sigmaT;
	int nx, ny, nz;
	float* density;
	float invMaxDensity;
	float3 p0, p1;
	int iterMax;

public:
	// Sometimes it will lead to timeout due to too deep iteration
	__device__ float3 Tr(const Ray& ray, thrust::uniform_real_distribution<float>& uniform, thrust::default_random_engine& rng) const {
		float sigma = dot(sigmaT, { 0.212671f, 0.715160f, 0.072169f });
		Ray r = ray;
		float3 d = p1 - p0;
		float tr = 1.f;
		float dist = 0.f;
		int iter = iterMax;

		// Perform ratio tracking to estimate the transmittance value
		while (true) {
			dist += -log(uniform(rng)) * invMaxDensity / sigma;
			if (dist >= r.tmax) break;
			float3 p = r(dist);
			p = (p - p0) / d;
			tr *= 1.f - getDensity(p) * invMaxDensity;

			// when transmittance gets low, start applying Russian roulette to terminate sampling.
			if (tr < 0.1f) {
				float q = 1.f - tr;
				if (uniform(rng) < q) return{ 0.f, 0.f, 0.f };
				tr /= (1.f - q);
			}

			if (--iter == 0)break;
		}
		return{ tr, tr, tr };
	}

	__device__ float3 Sample(const Ray& ray, thrust::uniform_real_distribution<float>& uniform, thrust::default_random_engine& rng, float& t, bool& sampled) const {
		float sigma = dot(sigmaT, { 0.212671f, 0.715160f, 0.072169f });
		Ray r = ray;
		float3 d = p1 - p0;
		float dist = 0.f;
		int iter = iterMax;

		// Run delta-tracking iterations to sample a medium interaction
		while (true) {
			dist += -log(uniform(rng)) * invMaxDensity / sigma;
			if (dist >= r.tmax) break;
			float3 p = r(dist);
			p = (p - p0) / d;
			if (getDensity(p) * invMaxDensity > uniform(rng)) {
				t = dist;
				sampled = true;
				return sigmaS / sigmaT;
			}

			if (--iter == 0)
				break;
		}

		t = dist;
		sampled = false;
		return make_float3(1.f, 1.f, 1.f);
	}

private:
	__device__ float getDensity(float3& p) const {
		float3 ps = make_float3(p.x * nx - .5f, p.y * ny - .5f, p.z * nz - .5f);
		float3 psi = make_float3(floor(ps.x), floor(ps.y), floor(ps.z));
		float3 delta = ps - psi;
		float d00 = lerp(d(psi), d(psi + make_float3(1, 0, 0)), delta.x);
		float d10 = lerp(d(psi + make_float3(0, 1, 0)), d(psi + make_float3(1, 1, 0)), delta.x);
		float d01 = lerp(d(psi + make_float3(0, 0, 1)), d(psi + make_float3(1, 0, 1)), delta.x);
		float d11 = lerp(d(psi + make_float3(0, 1, 1)), d(psi + make_float3(1, 1, 1)), delta.x);
		float d0 = lerp(d00, d10, delta.y);
		float d1 = lerp(d01, d11, delta.y);
		return lerp(d0, d1, delta.z);
	}

	__device__ float d(float3& p) const {
		int x = p.x, y = p.y, z = p.z;
		if (x<0 || x>nx - 1 || y<0 || y>ny - 1 || z<0 || z>nz - 1) return 0.f;
		int idx = z * ny * nx + y * nx + x;
		return density[idx];
	}
};

enum MediumType {
	MT_HOMOGENEOUS = 0,
	MT_HETEROGENEOUS,
};

class Medium {
public:
	MediumType type;
	float g;

	union {
		Homogeneous homogeneous;
		Heterogeneous heterogeneous;
	};

	__device__ void SamplePhase(float2 u, float3& dir, float& phase, float& pdf) const {
		if (g == 0) {
			phase = ONE_OVER_FOUR_PI;
			dir = UniformSampleSphere(u.x, u.y, pdf);
			return;
		}

		// HenyeyGreenstein Method Definitions
		float costheta;
		// Compute $\cos \theta$ for Henyey--Greenstein sample
		if (fabs(g) < 1e-3)
			costheta = 1.f - 2.f * u.x;
		else {
			float sqrtTerm = (1.f - g * g) / (1.f - g + 2.f * g * u.x);
			costheta = (1.f + g * g - sqrtTerm * sqrtTerm) / (2.f * g);
		}

		// Compute direction for Henyey--Greenstein sample
		float sintheta = sqrt(1.f - costheta * costheta);
		float phi = TWOPI * u.y;
		float sinphi = sin(phi), cosphi = cos(phi);
		dir = make_float3(sintheta * cosphi, sintheta * sinphi, costheta);
		float cubicTerm = (1.f + g * g - 2.f * g * costheta);
		// PhaseHG
		phase = ONE_OVER_FOUR_PI * (1.f - g * g) / (cubicTerm * sqrt(cubicTerm));
		pdf = phase;
	}

	__device__ void Phase(float3& in, float3& out, float& phase, float& pdf) {
		if (g == 0) {
			phase = ONE_OVER_FOUR_PI;
			pdf = phase;
			return;
		}

		// HenyeyGreenstein Method Definitions
		float costheta = dot(in, out);
		float cubicTerm = (1.f + g * g - 2.f * g * costheta);
		phase = ONE_OVER_FOUR_PI * (1.f - g * g) / sqrt(cubicTerm * cubicTerm * cubicTerm);
		pdf = phase;
	}
};

static void ReadDensityFromFile(const char* file, int nx, int ny, int nz, float* d) {
	FILE* fp = nullptr;
	fp = fopen(file, "r");
	for (int i = 0; i < nx * ny * nz; ++i) {
		float c;
		fscanf(fp, "%f\n", &c);
		d[i] = c;
	}
}