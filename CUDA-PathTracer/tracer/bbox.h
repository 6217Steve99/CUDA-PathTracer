#pragma once
#include "common.h"
#include "ray.h"

class BBox{
public:
	float3 fmin, fmax;

public:
	__host__ __device__ BBox(){
		fmin = make_float3(INFINITY, INFINITY, INFINITY);
		fmax = make_float3(-INFINITY, -INFINITY, -INFINITY);
	}

	__host__ __device__ BBox(float3& a, float3& b){
		fmin = a;
		fmax = b;
	}

	__host__ __device__ void Reset(){
		fmin = make_float3(INFINITY, INFINITY, INFINITY);
		fmax = make_float3(-INFINITY, -INFINITY, -INFINITY);
	}

	//expand from 2d case
	__host__ __device__ void Expand(BBox& b){
		fmin.x = fminf(b.fmin.x, fmin.x);
		fmin.y = fminf(b.fmin.y, fmin.y);
		fmin.z = fminf(b.fmin.z, fmin.z);

		fmax.x = fmaxf(b.fmax.x, fmax.x);
		fmax.y = fmaxf(b.fmax.y, fmax.y);
		fmax.z = fmaxf(b.fmax.z, fmax.z);
	}

	__host__ __device__ void Expand(float3& v){
		fmin.x = fminf(fmin.x, v.x);
		fmin.y = fminf(fmin.y, v.y);
		fmin.z = fminf(fmin.z, v.z);

		fmax.x = fmaxf(fmax.x, v.x);
		fmax.y = fmaxf(fmax.y, v.y);
		fmax.z = fmaxf(fmax.z, v.z);
	}

	__host__ __device__ float3 Centric() const{
		return (fmin + fmax)*0.5f;
	}

	__host__ __device__ float3 Diagonal() const{
		return fmax - fmin;
	}

	__host__ __device__ float3 Offset(float3 p) const{
		float3 diag = Diagonal();
		float3 delta = p - fmin;
		return delta / diag;
	}

	__host__ __device__ float SurfaceArea() const{
		float3 delta = fmax - fmin;
		return 2.f*(delta.x*delta.y + delta.y*delta.z + delta.z*delta.x);
	}

	__host__ __device__ int GetMaxExtent() const{
		float3 diag = Diagonal();
		if (diag.x > diag.y && diag.x > diag.z)
			return 0;
		else if (diag.y > diag.z)
			return 1;
		else
			return 2;
	}

	__host__ __device__ bool Intersect(Ray& r) const {
		//�ཻ����,���߽���ƽ������tֵС���뿪ƽ�����Сtֵ
		float3 inv_dir = { 1.f / r.destination.x, 1.f / r.destination.y, 1.f / r.destination.z };
		float txNear = (fmin.x - r.origin.x) * inv_dir.x;
		float txFar = (fmax.x - r.origin.x) * inv_dir.x;
		float tyNear = (fmin.y - r.origin.y) * inv_dir.y;
		float tyFar = (fmax.y - r.origin.y) * inv_dir.y;
		float tzNear = (fmin.z - r.origin.z) * inv_dir.z;
		float tzFar = (fmax.z - r.origin.z) * inv_dir.z;

		//���ཻtmin tmax��Ϊ����
		float tNear = fmaxf(fmaxf(fminf(txNear, txFar), fminf(tyNear, tyFar)), fminf(tzNear, tzFar));
		float tFar = fminf(fminf(fmaxf(txNear, txFar), fmaxf(tyNear, tyFar)), fmaxf(tzNear, tzFar));

		// Update _tFar_ to ensure robust ray--bounds intersection
		tFar *= 1 + 2 * gamma(3);

		if (tFar <= 0.00001f) return false;//��Χ�������ߺ�
		if (tNear > tFar)
			return false;
		if (tNear > r.tmax)
			return false;
		return true;
	}
	

	__host__ __device__ void boundingSphere(float3& center, float& radius) const{
		center = Centric();
		radius = sqrtf(dot(fmax - center, fmax - center));
	}
};