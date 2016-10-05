% ****************************************
%   Metodi del Calcolo Scientifico 2016
% ****************************************

clear all
clc

% ottengo la lista dei file .mat nella cartella
folderName = input('type folder name: ','s')
matfolder = what(folderName);
addpath(matfolder.path)
matlist = matfolder.mat;

for i = 1:(length(matlist))
    
    disp(['======================================']);

    % carico il file di dati ed ottengo il referirmento alla matrice dei
    % coefficienti A
    mat = load(char(matlist(i))); 
    A = mat.Problem.A; 
    
    disp(['Name matrix: ', mat.Problem.name]);
    disp(['Rows: ', num2str(size(A,2))]);
    disp(['Columns: ', num2str(size(A,1))]);
    disp(['Nonzero: ', num2str(nnz(A))]);
    disp(['-  -  -  -  -  -  -  -  -  -']);

    % abilito il profiler all'analisi della memoria
    profile clear
    profile -memory -history on

    % ottengo il vettore colonna unitario con numero righe di A
    xe = ones(length(A),1);
    % ottengo il vettore dei termini noti b = A * xe
    b = A * xe;
    
    tic
    % risoluzione del sistema lineare (con UMFPACK)
    x = A\b; 
    t = toc;
    disp(['Solving time: ',num2str(t),' s']);

    % calcolo errore relativo
    relErr = norm(x-xe)/norm(xe);
    disp(['Relative error: ',num2str(relErr)]);

    % genero il report dell'occupazione della memoria
    %profile viewer
    p = profile('info');
    mem = (p.FunctionTable(4).TotalMemAllocated)/1000;
    disp(['Allocated memory: ',num2str(mem),' kB']);
    
    % memorizzo i dati ottenuti in una lista
     listMatrix(i).name_matrix = mat.Problem.name;
     listMatrix(i).rows = size(A,2);
     listMatrix(i).solv_time = t;
     listMatrix(i).relative_error = relErr;
     listMatrix(i).alloc_memory = double(mem);
    
end

sortrows(struct2table(listMatrix),2)
