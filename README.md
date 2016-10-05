# mcs-sparsematrix

Exam project: Metodi del Calcolo Scientifico (2015/16)

### Project: First Part
**Target:** solve sparse matrix, calculate time and space required to solve, relative error and compare between different languages (Python and MATLAB)

**Matrix source:** [FEMLAB](http://www.cise.ufl.edu/research/sparse/matrices/FEMLAB/index.html) 

#### Python

Packages required:

```
import os
import scipy
import scipy.io
import psutil #(4.2.0)
from scipy.sparse import csc_matrix 
from scipy.sparse import linalg
from scipy.linalg import norm 
from datetime import datetime 
from memory_profiler import profile #(0.41)

 # install scikit-umfpack (0.2.1)
```

- `xSolver(A,b,bool)`: scipy solver for sparse matrix; A = matrix, b = constant based on system [ _b = A*xe_ ], bool = if True use UMFPACK library

- `@profile`: add this string before `xSolver` method to profile mamory used from it

- `matrixSolver(folder, eachmtx)`: function to solve all matrix (with format file *.mtx*) in selected folder, calculate time to solve with and without UMFPACK library and calculate relative error (based on equation  *relErr = (x-xe)/(xe)* with vector *xe = [1 1 1 1  ...]* )

- `loopSparseMatrix()`: allow to select folder with matrix file to solve

#### MATLAB

Solve linear system in MATLAB is very easy:
`x = A\b;` and it's done.

To calculate memory used solving add `profile -memory -history on` line.

(this code use matrix file ".mat")
 
