![CheMPS2 Logo](https://raw.githubusercontent.com/sebwouters/CheMPS2/master/CheMPS2/CheMPS2logo.png?raw=true)

CheMPS2: a spin-adapted implementation of DMRG for ab initio quantum chemistry
==============================================================================

Copyright (C) 2013-2015 Sebastian Wouters <sebastianwouters@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

Information
-----------

CheMPS2 is a scientific library which contains a spin-adapted implementation 
of the density matrix renormalization group (DMRG) for ab initio quantum 
chemistry. This method allows to obtain numerical accuracy in active spaces 
beyond the capabilities of full configuration interaction (FCI):

* up to 40 electrons in 40 orbitals for general active spaces
* up to 100 electrons in 100 orbitals for one-dimensional active spaces, such as the pi-system of all-trans polyenes

In addition, DMRG allows to obtain the 2-RDM of the active space efficiently. 
The method is therefore ideal to replace the FCI solver in the complete active 
space configuration interaction (CASCI) and complete active space self 
consistent field (CASSCF) methods, when the active space sizes become 
prohibitively expensive for FCI. The corresponding methods are called DMRG-CI 
and DMRG-SCF, respectively. Because DMRG can handle the abovementioned active 
space sizes, it allows to obtain FCI energies for small systems such as dimers, 
while for larger systems it is ideal to treat the static/strong correlation in 
a large active space.

The design philosophy for CheMPS2 is to be a lightweight, efficient, and 
stable library. For an input Hamiltonian and targeted symmetry sector, the 
library performs successive DMRG sweeps according to a user-defined 
convergence scheme. As output, the library returns the minimal encountered 
energy as well as the 2-RDM of the active space. With the latter, various 
molecular properties can be calculated, as well as the gradient and Hessian 
for orbital rotations or nuclear displacements. In addition, several 
correlation functions can be obtained to investigate the electronic structure 
in the active space.

To gain a better understanding of how to perform DMRG calculations for
large active spaces, you are encouraged to read the
[user manual](http://sebwouters.github.io/CheMPS2/index.html) and the
[three papers](#how-to-acknowledge-chemps2) listed below.

Incorporation of the library into other codes is very simple due a
minimal API, as well as a python interface. Direct usage of the
library is illustrated in

* the [c++](#4-test-libchemps2) tests
* the [python](#7-test-pychemps2) tests
* the [binary](#5-test-the-chemps2-binary) example

The interfaces to [psi4](http://www.psicode.org)
and [pyscf](https://github.com/sunqm/pyscf) are described in the
[user manual](http://sebwouters.github.io/CheMPS2/index.html).

CheMPS2 is parallelized for shared memory architectures with the Open
Multi-Processing ([OpenMP](http://openmp.org/wp/)) API and for
distributed memory architectures with the Message Passing Interface
([MPI](http://www.mpi-forum.org/)). A hybrid combination of both
parallelization strategies is supported. In the future,
the calculation of the 2-RDM of the active space will be extended
to the 3-RDM.


How to acknowledge CheMPS2
--------------------------

To acknowledge CheMPS2, please cite

1. S. Wouters, W. Poelmans, P.W. Ayers and D. Van Neck,
   CheMPS2: a free open-source spin-adapted implementation of the density
   matrix renormalization group for ab initio quantum chemistry,
   *Computer Physics Communications* **185** (6), 1501-1514 (2014),
   [doi:10.1016/j.cpc.2014.01.019](http://dx.doi.org/10.1016/j.cpc.2014.01.019)

        @article{CheMPS2cite1,
            author = {Sebastian Wouters and Ward Poelmans and Paul W. Ayers and Dimitri {Van Neck}},
            title = {CheMPS2: a free open-source spin-adapted implementation of the density matrix renormalization group for ab initio quantum chemistry},
            journal = {Computer Physics Communications},
            year = {2014},
            volume = {185},
            number = {6},
            pages = {1501-1514},
            doi = {10.1016/j.cpc.2014.01.019}
        }

2. S. Wouters and D. Van Neck,
   The density matrix renormalization group for ab initio quantum chemistry,
   *European Physical Journal D* **68** (9), 272 (2014),
   [doi:10.1140/epjd/e2014-50500-1](http://dx.doi.org/10.1140/epjd/e2014-50500-1)

        @article{CheMPS2cite2,
            author = {Sebastian Wouters and Dimitri {Van Neck}},
            title = {The density matrix renormalization group for ab initio quantum chemistry},
            journal = {European Physical Journal D},
            year = {2014},
            volume = {68},
            number = {9},
            pages = {272},
            doi = {10.1140/epjd/e2014-50500-1}
        }

3. S. Wouters, T. Bogaerts, P. Van Der Voort, V. Van Speybroeck and D. Van Neck,
   Communication: DMRG-SCF study of the singlet, triplet, and quintet states of oxo-Mn(Salen),
   *Journal of Chemical Physics* **140** (24), 241103 (2014),
   [doi:10.1063/1.4885815](http://dx.doi.org/10.1063/1.4885815)
   
        @article{CheMPS2cite3,
            author = {Sebastian Wouters and Thomas Bogaerts and Pascal {Van Der Voort} and Veronique {Van Speybroeck} and Dimitri {Van Neck}},
            title = {Communication: DMRG-SCF study of the singlet, triplet, and quintet states of oxo-Mn(Salen)},
            journal = {Journal of Chemical Physics},
            year = {2014},
            volume = {140},
            number = {24},
            pages = {241103},
            doi = {10.1063/1.4885815}
        }


Installation
------------

### 1. Dependencies

CheMPS2 can be built with [CMake](http://www.cmake.org/) and depends on

-   BLAS
-   LAPACK
-   GSL ([GNU Scientific library](http://www.gnu.org/software/gsl/))
-   HDF5 ([Hierarchical Data Format Release 5](http://www.hdfgroup.org/HDF5/))

It is parallelized for shared memory architectures with the Open
Multi-Processing ([OpenMP](http://openmp.org/wp/)) API and for
distributed memory architectures with the Message Passing Interface
([MPI](http://www.mpi-forum.org/)). A hybrid combination of both
parallelization strategies is supported.

### 2. Download

It is advised to clone the CheMPS2 git repository from github. In your
terminal, run:

    > cd /sourcefolder
    > git clone 'https://github.com/sebwouters/chemps2'
    > cd chemps2

That way, future updates and bug fixes can be easily pulled in:

    > cd /sourcefolder/chemps2
    > git pull

### 3. Build the chemps2 library and binary

The files

    /sourcefolder/chemps2/CMakeLists.txt
    /sourcefolder/chemps2/CheMPS2/CMakeLists.txt
    /sourcefolder/chemps2/tests/CMakeLists.txt
    /sourcefolder/chemps2/sphinx/CMakeLists.txt

provide a minimal compilation. In your terminal, run:

    > cd /sourcefolder/chemps2
    > mkdir build
    > cd build

CMake generates makefiles based on the user’s specifications:

    > CXX=option1 cmake .. -DMKL=option2 -DCMAKE_INSTALL_PREFIX=/option3 -DWITH_MPI=option4

1.  Option1 is the `c++` compiler; typically `g++`, `icpc`, or `clang++`
    on Linux. It is advised to use the intel compiler if available.
2.  Option2 can be `ON` or `OFF` and is used to switch on the intel math
    kernel library.
3.  /option3 is the prefix of the installation directory; typically `/usr`
    or `/usr/local` on Linux. On my computer, libchemps2 is then installed
    in `/option3/lib/x86_64-linux-gnu`, the headers in
    `/option3/include/chemps2`, and the binary in
    `/option3/bin/chemps2`.
4.  Option4 can be `ON` or `OFF` and is used to switch on the possibility
    to compile with MPI. Please note that the compiler should then provide
    `mpi.h`. Option1 should hence be the `mpic++` compiler; typically
    `mpic++` or `mpiicpc` on Linux. It is advised to use the intel compiler
    if available.

If one or more of the required libraries are not found, use the command

    > CMAKE_INCLUDE_PATH=option5 CMAKE_LIBRARY_PATH=option6 CXX=option1 cmake .. -DMKL=option2 -DCMAKE_INSTALL_PREFIX=/option3 -DWITH_MPI=option4

instead, where option5 and option6 are respectively the missing
colon-separated include and library paths:

    CMAKE_INCLUDE_PATH=/my_libs/lib1/include:/my_libs/lib2/include
    CMAKE_LIBRARY_PATH=/my_libs/lib1/lib:/my_libs/lib2/lib
    
For debian/sid, the HDF5 headers are located in the folder
`/usr/include/hdf5/serial`. If CMake complains about the HDF5 headers,
try to pass it with the option
`-DHDF5_INCLUDE_DIRS=/usr/include/hdf5/serial`.

To compile, run:

    > make

To install, run:

    > make install

For non-standard installation directories, please remember to append the
library path to `LD_LIBRARY_PATH` in your `.bashrc`.

### 4. Test libchemps2

To test libchemps2 for compilation **without MPI**, run:

    > cd /sourcefolder/chemps2/build
    > make test

To test libchemps2 for compilation **with MPI**, run:
    
    > cd /sourcefolder/chemps2/build/tests
    > OMP_NUM_THREADS=YYY mpirun -np ZZZ ./test1
    > OMP_NUM_THREADS=YYY mpirun -np ZZZ ./test2
    > OMP_NUM_THREADS=YYY mpirun -np ZZZ ./test3
    > OMP_NUM_THREADS=YYY mpirun -np ZZZ ./test4
    > OMP_NUM_THREADS=YYY mpirun -np ZZZ ./test5
    > OMP_NUM_THREADS=YYY mpirun -np ZZZ ./test7
    > OMP_NUM_THREADS=YYY mpirun -np ZZZ ./test10

`YYY` specifies the number of threads per process and `ZZZ` the number of
processes. The tests only require a very limited amount of memory (order
100 MB). Note that the tests are too small to see (near) linear scaling
with the number of cores, although improvement should still be noticeable.

### 5. Test the chemps2 binary

To test the chemps2 binary for compilation **without MPI**, run:

    > man /sourcefolder/chemps2/chemps2.1
    > cd /sourcefolder/chemps2/build/CheMPS2
    > ./chemps2 --help
    > ./chemps2 --fcidump=/sourcefolder/chemps2/tests/matrixelements/H2O.631G.FCIDUMP \
                --group=5 \
                --sweep_d=200,1000 \
                --sweep_econv=1e-8,1e-8 \
                --sweep_maxit=2,10 \
                --sweep_noise=0.05,0.0 \
                --twodmfile=2dm.out \
                --print_corr \
                --reorder=6,5,4,3,2,1,0,7,8,9,10,11,12
    
To test the chemps2 binary for compilation **with MPI**, prepend the binary with:

    > OMP_NUM_THREADS=YYY mpirun -np ZZZ ./chemps2 [OPTIONS]

### 6. Build PyCheMPS2

PyCheMPS2 is a python interface to libchemps2, for
compilation **without MPI**. It can be built with
[Cython](http://cython.org/). The installation is independent of
CMake and assumes that you have installed the CheMPS2 library with
`make install`. For non-standard installation directories of CheMPS2,
please remember to append the library path to `LD_LIBRARY_PATH` in
your `.bashrc`. In addition, the include path should be appended
to `CPATH`:

    > export CPATH=${CPATH}:/option3/include

where `/option3` is the option provided to CMake with
`-DCMAKE_INSTALL_PREFIX=/option3` above. For debian/sid, the HDF5
headers are located in the folder `/usr/include/hdf5/serial`. If it
was explicitly passed to CMake, it should also be appended to `CPATH`:

    > export CPATH=${CPATH}:/option3/include:/usr/include/hdf5/serial

The python wrapper can be installed with:

    > cd /sourcefolder/chemps2/PyCheMPS2
    > python setup.py build_ext -L ${LD_LIBRARY_PATH}
    > python setup.py install --prefix=/option3

On my machine, the python wrapper is installed to the folder
`/option3/lib/python2.7/site-packages`, but the folder `lib` and
the distribution of python can vary.

Compilation of PyCheMPS2 occurs by linking to the `c++` library in the
installation directory. The installation of PyCheMPS2 will fail if that
library is not properly installed. If you have pulled a newer version of
CheMPS2, please remember to reinstall the `c++` library first, before
reinstalling PyCheMPS2!


### 7. Test PyCheMPS2

When libchemps2 has been compiled **without MPI**, PyCheMPS2 can be tested
by running (remember that the python site-packages folder can vary):

    > cd /sourcefolder/chemps2/PyCheMPS2/tests
    > export PYTHONPATH=${PYTHONPATH}:/option3/lib/python2.7/site-packages
    > python test1.py
    > python test2.py
    > python test3.py
    > python test4.py
    > python test5.py
    > python test6.py
    > python test7.py
    > python test8.py
    > python test9.py
    > python test10.py

If you compiled the `c++` library with `-DMKL=ON`, you might get the error

    Intel MKL FATAL ERROR: Cannot load libmkl_avx.so or libmkl_def.so.

This issue of using Intel’s MKL inside python is known and reported. To
get the python tests to run, you can set the variable `LD_PRELOAD` in
order to preload libmkl\_rt.so. On my system, this is done with

    > export LD_PRELOAD=/opt/intel/mkl/lib/intel64/libmkl_rt.so

The python tests do exactly the same thing as the `c++` tests above, and
illustrate the usage of the python interface to libchemps2. The tests
should end with a line stating whether or not they succeeded. They only
require a very limited amount of memory (order 100 MB). Note that the
tests are too small to see (near) linear scaling with the number of cores,
although improvement should still be noticeable.


User manual
-----------

For information on how to perform DMRG and DMRG-SCF calculations with CheMPS2,
please consult the

1. [publications](#how-to-acknowledge-chemps2)
2. [user manual](http://sebwouters.github.io/CheMPS2/index.html)
3. [doxygen html output](http://sebwouters.github.io/CheMPS2/doxygen/index.html)

The [user manual](http://sebwouters.github.io/CheMPS2/index.html) contains
elaborate information on

* the DMRG and DMRG-SCF algorithms
* the symmetries which are exploited in CheMPS2
* how to generate matrix elements with plugins to [psi4](http://www.psicode.org)
* how to perform DMRG and DMRG-SCF calculations
* the interfaces of CheMPS2 to [psi4](http://www.psicode.org) and [pyscf](https://github.com/sunqm/pyscf)


List of files in CheMPS2
------------------------

[CheMPS2/CASSCF.cpp](CheMPS2/CASSCF.cpp) contains the constructor and
destructor of the CASSCF class, as well as the functions which allow to
update, get or set its variables.
    
[CheMPS2/CASSCFdebug.cpp](CheMPS2/CASSCFdebug.cpp) contains two
functions: one for calculating the ROHF energy; and one for fetching FCI
coefficients to determine the point group symmetry of certain electronic
states of the carbon dimer.
    
[CheMPS2/CASSCFnewtonraphson.cpp](CheMPS2/CASSCFnewtonraphson.cpp)
contains all DMRG-SCF functions which are specific to the augmented Hessian
Newton-Raphson update scheme, including the functions to calculate the
gradient and Hessian.
    
[CheMPS2/ConvergenceScheme.cpp](CheMPS2/ConvergenceScheme.cpp) contains
all functions of the ConvergenceScheme class, which contains the instructions
for the subsequent DMRG sweeps.

[CheMPS2/Correlations.cpp](CheMPS2/Correlations.cpp) contains all the
functionality to calculate the spin, density, and spin-flip correlation
functions as well as the two-orbital mutual information.

[CheMPS2/Davidson.cpp](CheMPS2/Davidson.cpp) is an implementation of
Davidson's algorithm.

[CheMPS2/DIIS.cpp](CheMPS2/DIIS.cpp) contains a DIIS convergence
speed-up for DMRG-SCF.

[CheMPS2/DMRG.cpp](CheMPS2/DMRG.cpp) contains the constructor and
destructor of the DMRG class, as well as the top-level sweep functions.

[CheMPS2/DMRGmpsio.cpp](CheMPS2/DMRGmpsio.cpp) contains the store and
load functions for the DMRG checkpoint file (the MPS).

[CheMPS2/DMRGoperators.cpp](CheMPS2/DMRGoperators.cpp) contains all
functions related to the DMRG renormalized operators: saving to disk, loading
from disk, and updating.

[CheMPS2/DMRGSCFindices.cpp](CheMPS2/DMRGSCFindices.cpp) contains the
index conversions for the DMRG-SCF algorithm.

[CheMPS2/DMRGSCFintegrals.cpp](CheMPS2/DMRGSCFintegrals.cpp) is a container
class for two-body matrix elements with at most two virtual indices.

[CheMPS2/DMRGSCFmatrix.cpp](CheMPS2/DMRGSCFmatrix.cpp) is a container
class for orbital matrices which are blockdiagonal in the irreps.

[CheMPS2/DMRGSCFoptions.cpp](CheMPS2/DMRGSCFoptions.cpp) is a container
class to pass the DMRGSCF options to the augmented Hessian Newton-Raphson
routine.

[CheMPS2/DMRGSCFunitary.cpp](CheMPS2/DMRGSCFunitary.cpp) contains the
storage and handling of the unitary matrix and its nonredundant skew-symmetric
parametrization required for the DMRG-SCF algorithm.

[CheMPS2/DMRGSCFVmatRotations.cpp](CheMPS2/DMRGSCFVmatRotations.cpp)
performs the two-body matrix element rotations for the DMRGSCF and
Edmiston-Ruedenberg classes.

[CheMPS2/DMRGSCFwtilde.cpp](CheMPS2/DMRGSCFwtilde.cpp) is a container
class to store a tensor which is required to compute the DMRG-SCF Hessian.

[CheMPS2/DMRGtechnics.cpp](CheMPS2/DMRGtechnics.cpp) contains the
functions related to the 2-RDM and the excited-state calculations.

[CheMPS2/EdmistonRuedenberg.cpp](CheMPS2/EdmistonRuedenberg.cpp) contains
an orbital localization function based on the Edmiston-Ruedenberg cost function
and an augmented Hessian Newton-Raphson optimizer.

[CheMPS2/FCI.cpp](CheMPS2/FCI.cpp) contains a full configuration
interaction solver based on Davidson's algorithm. It also contains the
functionality to calculate Green's functions.

[CheMPS2/FourIndex.cpp](CheMPS2/FourIndex.cpp) contains all functions of
the FourIndex container class for the two-body matrix elements.

[CheMPS2/Hamiltonian.cpp](CheMPS2/Hamiltonian.cpp) contains all functions
of the Hamiltonian class, including functions to get or set specific variables,
as well as the save and load functions to store the Hamiltonian on disk.

[CheMPS2/Heff.cpp](CheMPS2/Heff.cpp) contains the constructor and
destructor of the Heff class, as well as the top-level functions to perform
the effective Hamiltonian times guess-vector multiplication and Davidson's
algorithm.

[CheMPS2/HeffDiagonal.cpp](CheMPS2/HeffDiagonal.cpp) contains the
functions to calculate the diagonal elements of the effective Hamiltonian.
These are required as preconditioner in Davidson's algorithm.
    
[CheMPS2/HeffDiagrams1.cpp](CheMPS2/HeffDiagrams1.cpp) contains a subset
of functions to perform the effective Hamiltonian times guess-vector
multiplication.

[CheMPS2/HeffDiagrams2.cpp](CheMPS2/HeffDiagrams2.cpp) contains a subset
 of functions to perform the effective Hamiltonian times guess-vector
 multiplication.

[CheMPS2/HeffDiagrams3.cpp](CheMPS2/HeffDiagrams3.cpp) contains a subset
of functions to perform the effective Hamiltonian times guess-vector
multiplication.

[CheMPS2/HeffDiagrams4.cpp](CheMPS2/HeffDiagrams4.cpp) contains a subset
of functions to perform the effective Hamiltonian times guess-vector
multiplication.

[CheMPS2/HeffDiagrams5.cpp](CheMPS2/HeffDiagrams5.cpp) contains a subset
of functions to perform the effective Hamiltonian times guess-vector
multiplication.

[CheMPS2/Initialize.cpp](CheMPS2/Initialize.cpp) allows to set the seed
of the random number generator and cout.precision (added for PyCheMPS2).

[CheMPS2/Irreps.cpp](CheMPS2/Irreps.cpp) contains the psi4 symmetry
conventions.

[CheMPS2/PrintLicense.cpp](CheMPS2/PrintLicense.cpp) contains a function
which prints the license disclaimer.

[CheMPS2/Problem.cpp](CheMPS2/Problem.cpp) contains all Problem class
functions. This wrapper class allows to set the desired symmetry sector for
the DMRG algorithm.

[CheMPS2/Sobject.cpp](CheMPS2/Sobject.cpp) contains all Sobject class
functions. This class constructs, stores, and decomposes the reduced two-site
object.

[CheMPS2/SyBookkeeper.cpp](CheMPS2/SyBookkeeper.cpp) contains all
SyBookkeeper functions. This class keeps track of the FCI and DMRG virtual
dimensions of all symmetry sectors at all boundaries.

[CheMPS2/TensorA.cpp](CheMPS2/TensorA.cpp) contains the TensorA
functions. This class stores and handles the complementary reduced spin-0
S0-tensors.

[CheMPS2/TensorB.cpp](CheMPS2/TensorB.cpp) contains the TensorB
functions. This class stores and handles the complementary reduced spin-1
S1-tensors.

[CheMPS2/TensorC.cpp](CheMPS2/TensorC.cpp) contains the TensorC
functions. This class stores and handles the complementary reduced spin-0
F0-tensors.

[CheMPS2/TensorD.cpp](CheMPS2/TensorD.cpp) contains the TensorD
functions. This class stores and handles the complementary reduced spin-1
F1-tensors.

[CheMPS2/TensorDiag.cpp](CheMPS2/TensorDiag.cpp) contains the TensorDiag functions. This storage
class handles reduced tensors which are diagonal in the symmetry sectors.

[CheMPS2/TensorF0Cbase.cpp](CheMPS2/TensorF0Cbase.cpp) contains the common storage and handling
functions of the TensorF0 and TensorC classes.

[CheMPS2/TensorF0.cpp](CheMPS2/TensorF0.cpp) contains all TensorF0 functions. This class
stores and handles the reduced spin-0 part of two sandwiched second
quantized operators, of which the particle symmetry sectors are equal.

[CheMPS2/TensorF1.cpp](CheMPS2/TensorF1.cpp) contains all TensorF1 functions. This class
stores and handles the reduced spin-1 part of two sandwiched second
quantized operators, of which the particle symmetry sectors are equal.

[CheMPS2/TensorF1Dbase.cpp](CheMPS2/TensorF1Dbase.cpp) contains the common storage and handling
functions of the TensorF1 and TensorD classes.

[CheMPS2/TensorGYZ.cpp](CheMPS2/TensorGYZ.cpp) contains the contruct and update functions
for the G-, Y-, and Z-tensors. They are required for the two-orbital mutual
information.

[CheMPS2/TensorK.cpp](CheMPS2/TensorK.cpp) contains the contruct and update functions
for the K-tensor. It is required for the two-orbital mutual information.

[CheMPS2/TensorL.cpp](CheMPS2/TensorL.cpp) contains all TensorL functions. This class stores
and handles the reduced spin-1/2 part of a single sandwiched second quantized
operator.

[CheMPS2/TensorM.cpp](CheMPS2/TensorM.cpp) contains the contruct and update functions
for the M-tensor. It is required for the two-orbital mutual information.

[CheMPS2/TensorO.cpp](CheMPS2/TensorO.cpp) implements the storage and handling of the partial
terms which are required to calculate the overlap between two MPSs.

[CheMPS2/TensorQ.cpp](CheMPS2/TensorQ.cpp) contains all TensorQ functions. This class stores
and handles the complementary reduced spin-1/2 part of three sandwiched second
quantized operators.

[CheMPS2/TensorS0Abase.cpp](CheMPS2/TensorS0Abase.cpp) contains the common storage and handling
functions of the TensorS0 and TensorA classes.

[CheMPS2/TensorS0.cpp](CheMPS2/TensorS0.cpp) contains all TensorS0 functions. This class
stores and handles the reduced spin-0 part of two sandwiched second
quantized operators, of which the particle symmetry sectors differ by 2.

[CheMPS2/TensorS1Bbase.cpp](CheMPS2/TensorS1Bbase.cpp) contains the common storage and handling
functions of the TensorS1 and TensorB classes.

[CheMPS2/TensorS1.cpp](CheMPS2/TensorS1.cpp) contains all TensorS1 functions. This class
stores and handles the reduced spin-1 part of two sandwiched second
quantized operators, of which the particle symmetry sectors differ by 2.

[CheMPS2/TensorSwap.cpp](CheMPS2/TensorSwap.cpp) contains the common storage and handling
functions of the TensorL and TensorQ classes.

[CheMPS2/TensorT.cpp](CheMPS2/TensorT.cpp) contains all TensorT functions. This class
stores and handles the reduced part of an MPS site-tensor.

[CheMPS2/TensorX.cpp](CheMPS2/TensorX.cpp) contains all TensorX functions. This class
stores and handles the complementary reduced spin-0 part of four sandwiched
second quantized operators, which is of course diagonal in the symmetry
sectors.

[CheMPS2/TwoDM.cpp](CheMPS2/TwoDM.cpp) contains all functions to calculate the
2-RDM from the DMRG-optimized MPS.

[CheMPS2/TwoDMstorage.cpp](CheMPS2/TwoDMstorage.cpp) contains all functions to store the
2-RDM from the DMRG-optimized MPS.

[CheMPS2/TwoIndex.cpp](CheMPS2/TwoIndex.cpp) contains all functions of the TwoIndex container
class for the one-body matrix elements.

[CheMPS2/executable.cpp](CheMPS2/executable.cpp) builds to the chemps2 executable, which allows to use
libchemps2 from the command line.
    
[CheMPS2/include/chemps2/CASSCF.h](CheMPS2/include/chemps2/CASSCF.h) contains the definitions of the CASSCF class.

[CheMPS2/include/chemps2/ConvergenceScheme.h](CheMPS2/include/chemps2/ConvergenceScheme.h) contains the definitions of the
ConvergenceScheme class.

[CheMPS2/include/chemps2/Correlations.h](CheMPS2/include/chemps2/Correlations.h) contains the definitions of the
Correlations class.

[CheMPS2/include/chemps2/Davidson.h](CheMPS2/include/chemps2/Davidson.h) contains the definitions of the
Davidson class.

[CheMPS2/include/chemps2/DIIS.h](CheMPS2/include/chemps2/DIIS.h) contains the definitions of the DIIS class.

[CheMPS2/include/chemps2/DMRG.h](CheMPS2/include/chemps2/DMRG.h) contains the definitions of the DMRG class.

[CheMPS2/include/chemps2/DMRGSCFindices.h](CheMPS2/include/chemps2/DMRGSCFindices.h) contains the definitions of the
DMRGSCFindices class.

[CheMPS2/include/chemps2/DMRGSCFintegrals.h](CheMPS2/include/chemps2/DMRGSCFintegrals.h) contains the definitions of the
DMRGSCFintegrals class.

[CheMPS2/include/chemps2/DMRGSCFmatrix.h](CheMPS2/include/chemps2/DMRGSCFmatrix.h) contains the definitions of the
DMRGSCFmatrix class.

[CheMPS2/include/chemps2/DMRGSCFoptions.h](CheMPS2/include/chemps2/DMRGSCFoptions.h) contains the definitions of the
DMRGSCFoptions container class.

[CheMPS2/include/chemps2/DMRGSCFunitary.h](CheMPS2/include/chemps2/DMRGSCFunitary.h) contains the definitions of the
DMRGSCFunitary class.

[CheMPS2/include/chemps2/DMRGSCFVmatRotations.h](CheMPS2/include/chemps2/DMRGSCFVmatRotations.h) contains the definitions of the
DMRGSCFVmatRotations class.

[CheMPS2/include/chemps2/DMRGSCFwtilde.h](CheMPS2/include/chemps2/DMRGSCFwtilde.h) contains the definitions of the
DMRGSCFwtilde class.

[CheMPS2/include/chemps2/EdmistonRuedenberg.h](CheMPS2/include/chemps2/EdmistonRuedenberg.h) contains the definitions of the
EdmistonRuedenberg class.

[CheMPS2/include/chemps2/FCI.h](CheMPS2/include/chemps2/FCI.h) contains the definitions of the FCI class.

[CheMPS2/include/chemps2/FourIndex.h](CheMPS2/include/chemps2/FourIndex.h) contains the definitions of the FourIndex
class.

[CheMPS2/include/chemps2/Gsl.h](CheMPS2/include/chemps2/Gsl.h) contains the definitions of the external GSL
routines.

[CheMPS2/include/chemps2/Hamiltonian.h](CheMPS2/include/chemps2/Hamiltonian.h) contains the definitions of the Hamiltonian
class.

[CheMPS2/include/chemps2/Heff.h](CheMPS2/include/chemps2/Heff.h) contains the definitions of the Heff class.

[CheMPS2/include/chemps2/Initialize.h](CheMPS2/include/chemps2/Initialize.h) contains the definitions of the Initialize
class.

[CheMPS2/include/chemps2/Irreps.h](CheMPS2/include/chemps2/Irreps.h) contains the definitions of the Irrep class.

[CheMPS2/include/chemps2/Lapack.h](CheMPS2/include/chemps2/Lapack.h) contains the definitions of the external BLAS
and LAPACK routines.

[CheMPS2/include/chemps2/MPIchemps2.h](CheMPS2/include/chemps2/MPIchemps2.h) contains the
distribution of (complementary) renormalized operators over MPI processes, as well as
wrappers for the MPI communication routines in the C API.

[CheMPS2/include/chemps2/MyHDF5.h](CheMPS2/include/chemps2/MyHDF5.h) forces the use of the HDF5 1.8 API, e.g. 
H5Gcreate2 instead of H5Gcreate1, a known issue in Ubuntu 12.04.

[CheMPS2/include/chemps2/Options.h](CheMPS2/include/chemps2/Options.h) contains all the options of the CheMPS2
namespace. Here the checkpoint storage names and folders can be set, as well
as parameters related to memory usage and convergence.

[CheMPS2/include/chemps2/Problem.h](CheMPS2/include/chemps2/Problem.h) contains the definitions of the Problem class.

[CheMPS2/include/chemps2/Sobject.h](CheMPS2/include/chemps2/Sobject.h) contains the definitions of the Sobject class.

[CheMPS2/include/chemps2/SyBookkeeper.h](CheMPS2/include/chemps2/SyBookkeeper.h) contains the definitions of the
SyBookkeeper class.

[CheMPS2/include/chemps2/TensorA.h](CheMPS2/include/chemps2/TensorA.h) contains the definitions of the TensorA class.

[CheMPS2/include/chemps2/TensorB.h](CheMPS2/include/chemps2/TensorB.h) contains the definitions of the TensorB class.

[CheMPS2/include/chemps2/TensorC.h](CheMPS2/include/chemps2/TensorC.h) contains the definitions of the TensorC class.

[CheMPS2/include/chemps2/TensorD.h](CheMPS2/include/chemps2/TensorD.h) contains the definitions of the TensorD class.

[CheMPS2/include/chemps2/TensorDiag.h](CheMPS2/include/chemps2/TensorDiag.h) contains the definitions of the
TensorDiag class.

[CheMPS2/include/chemps2/TensorF0Cbase.h](CheMPS2/include/chemps2/TensorF0Cbase.h) contains the definitions of the
TensorF0Cbase class.

[CheMPS2/include/chemps2/TensorF0.h](CheMPS2/include/chemps2/TensorF0.h) contains the definitions of the TensorF0 class.

[CheMPS2/include/chemps2/TensorF1Dbase.h](CheMPS2/include/chemps2/TensorF1Dbase.h) contains the definitions of the
TensorF1Dbase class.

[CheMPS2/include/chemps2/TensorF1.h](CheMPS2/include/chemps2/TensorF1.h) contains the definitions of the TensorF1 class.

[CheMPS2/include/chemps2/TensorGYZ.h](CheMPS2/include/chemps2/TensorGYZ.h) contains the definitions of the TensorGYZ
class.

[CheMPS2/include/chemps2/Tensor.h](CheMPS2/include/chemps2/Tensor.h) contains the definitions of the virtual Tensor
class.

[CheMPS2/include/chemps2/TensorK.h](CheMPS2/include/chemps2/TensorK.h) contains the definitions of the TensorK class.

[CheMPS2/include/chemps2/TensorL.h](CheMPS2/include/chemps2/TensorL.h) contains the definitions of the TensorL class.

[CheMPS2/include/chemps2/TensorM.h](CheMPS2/include/chemps2/TensorM.h) contains the definitions of the TensorM class.

[CheMPS2/include/chemps2/TensorO.h](CheMPS2/include/chemps2/TensorO.h) contains the definitions of the TensorO class.

[CheMPS2/include/chemps2/TensorQ.h](CheMPS2/include/chemps2/TensorQ.h) contains the definitions of the TensorQ class.

[CheMPS2/include/chemps2/TensorS0Abase.h](CheMPS2/include/chemps2/TensorS0Abase.h) contains the definitions of the
TensorS0Abase class.

[CheMPS2/include/chemps2/TensorS0.h](CheMPS2/include/chemps2/TensorS0.h) contains the definitions of the TensorS0 class.

[CheMPS2/include/chemps2/TensorS1Bbase.h](CheMPS2/include/chemps2/TensorS1Bbase.h) contains the definitions of the
TensorS1Bbase class.

[CheMPS2/include/chemps2/TensorS1.h](CheMPS2/include/chemps2/TensorS1.h) contains the definitions of the TensorS1 class.

[CheMPS2/include/chemps2/TensorSwap.h](CheMPS2/include/chemps2/TensorSwap.h) contains the definitions of the TensorSwap
class.

[CheMPS2/include/chemps2/TensorT.h](CheMPS2/include/chemps2/TensorT.h) contains the definitions of the TensorT class.

[CheMPS2/include/chemps2/TensorX.h](CheMPS2/include/chemps2/TensorX.h) contains the definitions of the TensorX class.

[CheMPS2/include/chemps2/TwoDM.h](CheMPS2/include/chemps2/TwoDM.h) contains the definitions of the TwoDM class.

[CheMPS2/include/chemps2/TwoDMstorage.h](CheMPS2/include/chemps2/TwoDMstorage.h) contains the definitions of the
TwoDMstorage class.

[CheMPS2/include/chemps2/TwoIndex.h](CheMPS2/include/chemps2/TwoIndex.h) contains the definitions of the TwoIndex
class.

Please note that these files are documented with doxygen comments. The
[doxygen html output](http://sebwouters.github.io/CheMPS2/doxygen/index.html)
can be consulted online.


List of files to perform test runs
----------------------------------

[tests/test1.cpp.in](tests/test1.cpp.in) contains several DMRG ground
state calculations in different symmetry sectors for the N2 molecule (d2h
symmetry) in the minimal STO-3G basis set.

[tests/test2.cpp.in](tests/test2.cpp.in) contains a ground state DMRG
calculation of the ^1A1 state of H2O (c2v symmetry) in the 6-31G basis set.

[tests/test3.cpp.in](tests/test3.cpp.in) contains a ground state DMRG
calculation of the ^1A1 state of CH4 (c2v symmetry) in the STO-3G basis set.

[tests/test4.cpp.in](tests/test4.cpp.in) contains a ground state DMRG
calculation of the ^6A state of a linear Hubbard chain (forced c1 symmetry)
with 10 sites and open boundary conditions, containing 9 fermions (just below
half-filling).

[tests/test5.cpp.in](tests/test5.cpp.in) contains an excited state DMRG
calculation in the ^1Ag symmetry sector of N2 (d2h symmetry) in the minimal
STO-3G basis set. The ground and two lowest excited states are determined.

[tests/test6.cpp.in](tests/test6.cpp.in) contains a state-averaged
DMRG-SCF calculation of the first excited state of the ^1Ag sector of O2 (d2h
symmetry) in the CC-pVDZ basis set. The two 1s core orbitals are kept frozen,
and two Ag, B2g, B3g, B1u, B2u, and B3u orbitals are chosen as active space. A
significant speedup is obtained with DIIS.

[tests/test7.cpp.in](tests/test7.cpp.in) reads in
[tests/matrixelements/O2.CCPVDZ.FCIDUMP](tests/matrixelements/O2.CCPVDZ.FCIDUMP),
stores these matrix elements to disk, reads them back in from disk, and
compares the two versions.

[tests/test8.cpp.in](tests/test8.cpp.in) contains a DMRG-SCF ground state
calculation of the ^1Ag state of N2 (d2h symmetry) in the CC-pVDZ basis set.
The two 1s core orbitals are kept frozen. The next two Ag and B1u orbitals
(sigma bonding and antibonding), as well as one B2g, B3g, B2u, and B3u orbital
(pi bonding and antibonding) are chosen as active space. A significant speedup
is obtained with DIIS. This test is smaller than test6, and is included for
debugging with valgrind.

[tests/test9.cpp.in](tests/test9.cpp.in) is a copy of
[tests/test8.cpp.in](tests/test8.cpp.in), with a slightly larger active
space and which works with ordered localized orbitals instead of natural
orbitals. The localization occurs by means of Edmiston-Ruedenberg, and the
ordering based on the Fiedler vector with the exchange matrix as cost function.

[tests/test10.cpp.in](tests/test10.cpp.in) contains a ground state DMRG
calculation of a half-filled square 3 by 3 Hubbard lattice, both in the site
basis and in the momentum basis. For the latter, the matrix elements only have
fourfold permutation symmetry.

[tests/matrixelements/CH4.STO3G.FCIDUMP](tests/matrixelements/CH4.STO3G.FCIDUMP)
contains the matrix elements for test3.

[tests/matrixelements/H2O.631G.FCIDUMP](tests/matrixelements/H2O.631G.FCIDUMP)
contains the matrix elements for test2.

[tests/matrixelements/N2.STO3G.FCIDUMP](tests/matrixelements/N2.STO3G.FCIDUMP)
contains the matrix elements for test1 and test5.

[tests/matrixelements/O2.CCPVDZ.FCIDUMP](tests/matrixelements/O2.CCPVDZ.FCIDUMP)
contains the matrix elements for test6 and test7.

[tests/matrixelements/N2.CCPVDZ.FCIDUMP](tests/matrixelements/N2.CCPVDZ.FCIDUMP)
contains the matrix elements for test8 and test9.

[PyCheMPS2/tests/test1.py](PyCheMPS2/tests/test1.py) is the python version of
[tests/test1.cpp.in](tests/test1.cpp.in)

[PyCheMPS2/tests/test2.py](PyCheMPS2/tests/test2.py) is the python version of
[tests/test2.cpp.in](tests/test2.cpp.in)

[PyCheMPS2/tests/test3.py](PyCheMPS2/tests/test3.py) is the python version of
[tests/test3.cpp.in](tests/test3.cpp.in)

[PyCheMPS2/tests/test4.py](PyCheMPS2/tests/test4.py) is the python version of
[tests/test4.cpp.in](tests/test4.cpp.in)

[PyCheMPS2/tests/test5.py](PyCheMPS2/tests/test5.py) is the python version of
[tests/test5.cpp.in](tests/test5.cpp.in)

[PyCheMPS2/tests/test6.py](PyCheMPS2/tests/test6.py) is the python version of
[tests/test6.cpp.in](tests/test6.cpp.in)

[PyCheMPS2/tests/test7.py](PyCheMPS2/tests/test7.py) is the python version of
[tests/test7.cpp.in](tests/test7.cpp.in)

[PyCheMPS2/tests/test8.py](PyCheMPS2/tests/test8.py) is the python version of
[tests/test8.cpp.in](tests/test8.cpp.in)

[PyCheMPS2/tests/test9.py](PyCheMPS2/tests/test9.py) is the python version of
[tests/test9.cpp.in](tests/test9.cpp.in)

[PyCheMPS2/tests/test10.py](PyCheMPS2/tests/test10.py) is the python version of
[tests/test10.cpp.in](tests/test10.cpp.in)

These test files illustrate how to use libchemps2. The tests only
require a very limited amount of memory (order 100 MB). Note that the
tests are too small to see (near) linear scaling with the number of cores,
although improvement should still be noticeable.

