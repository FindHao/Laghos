// Copyright (c) 2010, Lawrence Livermore National Security, LLC. Produced at
// the Lawrence Livermore National Laboratory. LLNL-CODE-443211. All Rights
// reserved. See file COPYRIGHT for details.
//
// This file is part of the MFEM library. For more information and source code
// availability see http://mfem.org.
//
// MFEM is free software; you can redistribute it and/or modify it under the
// terms of the GNU Lesser General Public License (as published by the Free
// Software Foundation) version 2.1 dated February 1999.
#include "raja.hpp"

namespace mfem {
  
RajaVector::~RajaVector(){
  if (!own) return;
  //rdbg("\033[33m[~v(%d)",size);
  rdbg("\033[33m[~v");
  rmalloc<double>::_delete(data);
}

// ***************************************************************************
double* RajaVector::alloc(const size_t sz) {
  rdbg("\033[33m[v");
  return (double*) rmalloc<double>::_new(sz);
}

// ***************************************************************************
void RajaVector::SetSize(const size_t sz, const void* ptr) {
  //rdbg("\033[33m[size=%d, new sz=%d]\033[m",size,sz);
  own=true;
  size = sz;
  if (!data) { data = alloc(sz); }
#ifdef __NVCC__
  if (ptr) { checkCudaErrors(cuMemcpyDtoD((CUdeviceptr)data,(CUdeviceptr)ptr,bytes()));}
#else
  if (ptr) { ::memcpy(data,ptr,bytes());}
#endif
}

// ***************************************************************************
RajaVector::RajaVector(const size_t sz):size(sz),data(alloc(sz)),own(true) {}
  RajaVector::RajaVector(const size_t sz,double value):
    size(sz),data(alloc(sz)),own(true) { *this=value;}

RajaVector::RajaVector(const RajaVector& v):
  size(0),data(NULL),own(true) { SetSize(v.Size(), v); }

RajaVector::RajaVector(const RajaVectorRef& ref):
  size(ref.v.size),data(ref.v.data),own(false) {}
  
RajaVector::RajaVector(RajaArray<double>& v):
  size(v.size()),data(v.ptr()),own(false) {}

// Host 2 Device ***************************************************************
RajaVector::RajaVector(const Vector& v):
  size(v.Size()),data(NULL),own(true) {
#ifdef __NVCC__
  //printf("\033[31m[RajaVector()] Host 2 Device\033[m\n");
  checkCudaErrors(cuMemAlloc((CUdeviceptr*)&data, size*sizeof(double)));
  checkCudaErrors(cuMemcpyHtoD((CUdeviceptr)data,v.GetData(),v.Size()*sizeof(double)));
#else
  SetSize(v.Size(), v.GetData());  
#endif
}

// Device 2 Host ***************************************************************
RajaVector::operator Vector() {
#ifdef __NVCC__
  //printf("\033[31m[Vector()] Device 2 Host\033[m\n");
  double *h_data= (double*) ::malloc(bytes());
  checkCudaErrors(cuMemcpyDtoH(h_data,(CUdeviceptr)data,bytes()));
  return Vector(h_data,size);
#else
  return Vector(data,size);
#endif
}
  
RajaVector::operator Vector() const {
#ifdef __NVCC__
  //printf("\033[31m[Vector()const] Device 2 Host\033[m\n");
  double *h_data= (double*) ::malloc(bytes());
  checkCudaErrors(cuMemcpyDtoH(h_data,(CUdeviceptr)data,bytes()));
  return Vector(h_data,size);
#else
  return Vector(data,size);
#endif
}

// ***************************************************************************
void RajaVector::Print(std::ostream& out, int width) const {
#ifdef __NVCC__
  //dbg()<<"Device 2 Host (const)";
  double *h_data= (double*) ::malloc(bytes());
  checkCudaErrors(cuMemcpyDtoH(h_data,(CUdeviceptr)data,bytes()));
#else
  double *h_data=data;
#endif
  for (size_t i=0; i<size; i+=1) 
    printf("\n\t[%ld] %.15e",i,h_data[i]);
}

// ***************************************************************************
RajaVectorRef RajaVector::GetRange(const size_t offset,
                                   const size_t entries) const {
  RajaVectorRef ret;
  RajaVector& v = ret.v;
  v.data = (double*) ((unsigned char*)data + (offset*sizeof(double)));
  v.size = entries;
  v.own = false;
  return ret;
}

// ***************************************************************************
RajaVector& RajaVector::operator=(const RajaVector& v) {
  SetSize(v.Size(),v.data);
  own = false;
  return *this;
}

// ***************************************************************************
RajaVector& RajaVector::operator=(double value) {
  vector_op_eq(size, value, data);
  return *this;
}

// ***************************************************************************
double RajaVector::operator*(const RajaVector& v) const {
  return vector_dot(size, data, v.data);
}

// *****************************************************************************
RajaVector& RajaVector::operator-=(const RajaVector& v) {
  vector_vec_sub(size, data, v.data);
  return *this;
}

// ***************************************************************************
RajaVector& RajaVector::operator+=(const RajaVector& v) {
  vector_vec_add(size, data, v.data);
  return *this;
}

// ***************************************************************************
RajaVector& RajaVector::operator*=(const double d) {
  vector_vec_mul(size, data, d);
  return *this;
}

// ***************************************************************************
RajaVector& RajaVector::Add(const double alpha, const RajaVector& v) {
  vector_axpy(Size(),alpha, data, v.data);
  return *this;
}


// ***************************************************************************
void RajaVector::Neg() {
  vector_neg(Size(),ptr());
}

// *****************************************************************************
void RajaVector::SetSubVector(const RajaArray<int> &ess_tdofs,
                              const double value,
                              const int N) {
  vector_set_subvector_const(N, value, data, ess_tdofs.ptr());
}


// ***************************************************************************
double RajaVector::Min() const {
  return vector_min(Size(),(double*)data);
}

// ***************************************************************************
// from mfem::TCGSolver<mfem::RajaVector>::Mult in linalg/solvers.hpp:224
void add(const RajaVector& v1, const double alpha,
         const RajaVector& v2, RajaVector& out) {
  vector_xpay(out.Size(),alpha,out.ptr(),v1.ptr(),v2.ptr());
}

// *****************************************************************************
void add(const double alpha,
         const RajaVector& v1,
         const double beta,
         const RajaVector& v2,
         RajaVector& out) {
  /* used in templated TRK3SSPSolver, but not here */
  assert(false);
}

// ***************************************************************************
void subtract(const RajaVector& v1,
              const RajaVector& v2,
              RajaVector& out) {
  vector_xsy(out.Size(),out.ptr(),v1.ptr(),v2.ptr());
}

} // mfem
