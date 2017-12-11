// Copyright (c) 2017, Lawrence Livermore National Security, LLC. Produced at
// the Lawrence Livermore National Laboratory. LLNL-CODE-734707. All Rights
// reserved. See files LICENSE and NOTICE for details.
//
// This file is part of CEED, a collection of benchmarks, miniapps, software
// libraries and APIs for efficient high-order finite element and spectral
// element discretizations for exascale applications. For more information and
// source code availability see http://github.com/ceed.
//
// The CEED research is supported by the Exascale Computing Project 17-SC-20-SC,
// a collaborative effort of two U.S. Department of Energy organizations (Office
// of Science and the National Nuclear Security Administration) responsible for
// the planning and preparation of a capable exascale ecosystem, including
// software, applications, hardware, advanced system engineering and early
// testbed platforms, in support of the nation's exascale computing imperative.
#ifndef MFEM_RAJA_ARRAY
#define MFEM_RAJA_ARRAY

namespace mfem {

template <class T, bool xyz = true> class RajaArray;

// Partial Specializations for xyz==TRUE *************************************
  template <class T> class RajaArray<T,true> : public rmanaged<T,mng>{
 private:
  T* data = NULL;
  size_t sz,d[4];
 public:
  RajaArray():data(NULL),sz(0),d{0,0,0,0} {}
  RajaArray(const size_t x) {allocate(x);}
  RajaArray(const size_t x,const size_t y) {allocate(x,y);}
  RajaArray(const RajaArray<T,true> &r) {assert(false);}
 ~RajaArray(){printf("\033[32mx\033[m");fflush(stdout);this->operator delete(data);}
  inline T* ptr() { return data; }
  inline const T* ptr() const { return data; }
  inline operator T* () { return data; }
  inline operator const T* () const { return data; }
  inline size_t size() const { return sz; }
  inline size_t bytes() const { return size()*sizeof(T); }
  void allocate(const size_t X, const size_t Y =1,
                const size_t Z =1, const size_t D =1,
                const bool transposed = false) {
    d[0]=X; d[1]=Y; d[2]=Z; d[3]=D;
    sz=d[0]*d[1]*d[2]*d[3];
    printf("\033[32mi\033[m");fflush(stdout);
    data=(T*) this->operator new(sz*sizeof(T));
  }
  inline T& operator[](const size_t x) { return data[x]; }
  inline T& operator()(const size_t x, const size_t y) {
    return data[x + d[0]*y];
  }
  inline T& operator()(const size_t x, const size_t y, const size_t z) {
    return data[x + d[0]*(y + d[1]*z)];
  }
};

// Partial Specializations for xyz==FALSE ************************************
  template <class T> class RajaArray<T,false> : public rmanaged<T,mng>{
 private:
  static const int DIM = 4;
  T* data = NULL;
  size_t sz,d[DIM];
 public:
  RajaArray():data(NULL),sz(0),d{0,0,0,0} {}
  RajaArray(const size_t d0) {allocate(d0);}
  RajaArray(const RajaArray<T,false> &r) {assert(false);}
 ~RajaArray(){this->operator delete(data);}
  inline T* ptr() { return data; }
  inline const T* ptr() const { return data; }
  inline operator T* () { return data; }
  inline operator const T* () const { return data; }
  inline size_t size() const { return sz; }
  inline size_t bytes() const { return size()*sizeof(T); }
  void allocate(const size_t X, const size_t Y =1,
                const size_t Z =1, const size_t D =1,
                const bool transposed = false) {
    d[0]=X; d[1]=Y; d[2]=Z; d[3]=D;
    sz=d[0]*d[1]*d[2]*d[3];
    printf("\033[32mI\033[m");fflush(stdout);
    data=(T*) this->operator new(sz*sizeof(T));
#define xsw(a,b) a^=b^=a^=b
    if (transposed) { xsw(d[0],d[1]); }
    for (size_t i=1,b=d[0]; i<DIM; xsw(d[i],b),++i) {
      d[i]*=d[i-1];
    }
    d[0]=1;
    if (transposed) { xsw(d[0],d[1]); }
  }
  inline T& operator[](const size_t x) { return data[x]; }
  inline T& operator()(const size_t x, const size_t y) {
    return data[d[0]*x + d[1]*y];
  }
  inline T& operator()(const size_t x, const size_t y, const size_t z) {
    return data[d[0]*x + d[1]*y + d[2]*z];
  }
};

} // mfem

#endif // MFEM_RAJA_ARRAY

