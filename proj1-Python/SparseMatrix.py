
# ****************************************
#   Metodi del Calcolo Scientifico 2016
# ----------------------------------------
#   743464 Banfi Alessandro
#   735722 Curatitoli Mattia
#   742752 Ghelfi Camillo
# ****************************************

# *****************************************************
# Per una corretta esecuzione installare i pacchetti:
# - scikit-umfpack (0.2.1)
# - memory_profiler (0.41)
# - psutil (4.2.0)
# *****************************************************

# package per interfacciarsi col sistema operativo
import os as os
# package scipy con moduli per calcoli matematici
import scipy as scipy 
# modulo input/output files
import scipy.io as scipyio
# modulo matrici Compressed Sparse Column
from scipy.sparse import csc_matrix 
# modulo per algebra lineare
from scipy.sparse import linalg 
# modulo per calcolo della norma in matrici sparse
from scipy.linalg import norm 
# modulo per il timestamp
from datetime import datetime 
# modulo profiler memoria
from memory_profiler import profile
import psutil

# file di log della memoria rilevata dal profiler
# mem_prof = open('memory_profiler.log','w+')
# @profile(stream=mem_prof)
@profile
def xSolver(A,b,bool):
	x = scipy.sparse.linalg.spsolve(A,b,use_umfpack=bool)
	return x


def matrixSolver(folder, eachmtx):
	print '======================================'
	# ottengo il percorso di ogni file .mtx
	pathmtx = folder + '/' + eachmtx
	# leggo le info del file .mtx con scipyio.mminfo()
	mminfo = scipyio.mminfo(pathmtx)
	# scipyio.mminfo() = [rows, cols, entries, format, field, symmetry]
	# -> mminfo[0] = numero righe della matrice
	rows = mminfo[0] 
	# carico file .mtx e converto in matrice csc (Compressed Sparse Column) 
	A = scipyio.mmread(pathmtx).tocsc()
	# ottengo elementi diversi da zero in A
	nonzero = A.nnz

	print 'Name matrix: ' + pathmtx
	print 'Rows: ' + str(rows)
	print 'Columns: ' + str(rows)
	print 'Nonzero: ' + str (nonzero)
	print '-  -  -  -  -  -  -  -  -  -'

	# ottengo il vettore colonna unitario xe con numero righe di A
	xe = scipy.ones(rows) 
	# ottengo il vettore dei termini noti b = A * xe
	b = A * xe 
	# for i in range(0,10):
	# 	print format(b[i], '.30f')
	
	# inizializzo il vettore soluzione con il numero righe di A
	x = scipy.empty(rows) 
	timerOn = datetime.now() 
	# risoluzione del sistema lineare senza UMFPACK
	x = xSolver(A,b,False)
	t = datetime.now() - timerOn
	print 'Solving time without UMFPACK: ' + str(t) + ' s'

	# reinizializzo il vettore soluzione con il numero righe di A
	x = scipy.empty(rows) 
	timerOn = datetime.now() 
	# risoluzione del sistema lineare con UMFPACK
	x = xSolver(A,b,True)
	t = (datetime.now() - timerOn)
	print 'Solving time with UMFPACK: ' + str(t) + ' s'

	# calcolo errore relativo
	relErr = norm(x-xe)/norm(xe)
	print 'Relative error: ' + str(relErr)


def loopSparseMatrix(f):
	folder = f
	if os.path.isdir(folder): 
		path = os.path.abspath(folder)
		# elenco tutti i file .mtx contenuti nella cartella indicata in 'folder'
		mtxlist = [each for each in os.listdir(path) if each.endswith('.mtx')]

		for eachmtx in mtxlist:

			matrixSolver(folder, eachmtx)


if __name__ == '__main__':
  # funzione per la soluzione di tutte le matrici nella cartella indicata
  loopSparseMatrix('FEMLAB')
  
  # funzioni per la soluzione di ciascuna singola matrice
  # matrixSolver('FEMLAB', 'ns3Da.mtx')
  # matrixSolver('FEMLAB', 'poisson2D.mtx')
  # matrixSolver('FEMLAB', 'poisson3Da.mtx')
  # matrixSolver('FEMLAB', 'poisson3Db.mtx')
  # matrixSolver('FEMLAB', 'problem1.mtx')
  # matrixSolver('FEMLAB', 'sme3Da.mtx')
  # matrixSolver('FEMLAB', 'sme3Db.mtx')
  # matrixSolver('FEMLAB', 'sme3Dc.mtx')


