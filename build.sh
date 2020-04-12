# Tested for GCC >= 6.4.0
export MPI_HOME=`echo $MPI_HOME`
export CUDA_HOME=`echo $CUDA_HOME`

cd ../

# hypre
wget https://github.com/LLNL/hypre/archive/v2.11.2.tar.gz
tar -xvf v2.11.2.tar.gz
cd hypre-2.11.2/src
./configure --disable-fortran --with-MPI --with-MPI-include=$MPI_HOME/include --with-MPI-lib-dirs=$MPI_HOME/lib
make -j8
cd ../..

# metis
wget http://glaros.dtc.umn.edu/gkhome/fetch/sw/metis/metis-5.1.0.tar.gz
tar xzvf metis-5.1.0.tar.gz
cd metis-5.1.0
make config prefix=`pwd`
make -j8 && make install
cd ..

# mfem
git clone git@github.com:mfem/mfem.git
cd mfem
git checkout laghos-v2.0
make config MFEM_DEBUG=YES MFEM_USE_MPI=YES HYPRE_DIR=`pwd`/../hypre-2.11.2/src/hypre MFEM_USE_METIS_5=YES METIS_DIR=`pwd`/../metis-5.1.0
make status
make -j8
cd ..


# CUDA Laghos
cd Laghos/cuda
make debug NV_ARCH=-arch=sm_70 CUDA_DIR=$CUDA_HOME MPI_HOME=$MPI_HOME -j8
cd ../..

# run test
./laghos -p 0 -m ../..//Laghos/data/square01_quad.mesh --sync -rs 1 -tf 0.2 -pa