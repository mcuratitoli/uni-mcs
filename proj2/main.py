#!/usr/bin/python
# -*- coding: utf-8 -*-

# ****************************************
#   Metodi del Calcolo Scientifico 2016
# ----------------------------------------
#   743464 Banfi Alessandro
#   735722 Curatitoli Mattia
#   742752 Ghelfi Camillo
# ****************************************

from Tkinter import *
import ttk
import tkFileDialog
import tkMessageBox as mbox
import sys 
import os.path
import matplotlib
matplotlib.use("TkAgg")
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg, NavigationToolbar2TkAgg
from matplotlib.figure import Figure
from scipy.misc import imread
import matplotlib.pyplot as plt
import pylab
import numpy as np
import dct

root = Tk()
root.title(sys.argv[0])
root.columnconfigure(0, weight=1)

subframe1 = ttk.Frame(root)
subframe1.pack(side=TOP, fill=X)

label1 = ttk.Label(subframe1, text="Immagine sorgente:")
label1.pack(side=LEFT, padx=5, pady=5)

fileCnt = StringVar()
fileCnt.set('clicca qui per selezionarla')
file = ttk.Entry(subframe1, textvariable=fileCnt, state=DISABLED)

select1Cnt = StringVar()
select1Cnt.set(50)

src_file = ''
def choose_src_img(*args):
    global src_file
    types = [('BMP', '*.bmp'), ('Tutti i file', '*.*')]
    source = tkFileDialog.askopenfilename(title='Scelta dell\'immagine sorgente', filetypes=types)
    if source != '' and os.path.isfile(source):
        file["justify"] = RIGHT
        fileCnt.set(os.path.split(source)[1])
        src_file = source

file.bind('<Button-1>', choose_src_img)
file.pack(side=LEFT, pady=5)

subframe2 = ttk.Frame(root)
subframe2.pack(side=TOP, fill=X)

label2 = ttk.Label(subframe2, text="Qualità:")
label2.pack(side=LEFT, padx=5, pady=5)
select1Cnt = StringVar()

qPrev = "50"
def validate_quality(*args):
    global qPrev
    try:
        q = int(select1Cnt.get())
        if (q < 1) or (q > 100):
            select1Cnt.set(qPrev)
            return False
        qPrev = str(q)
    except ValueError:
        select1Cnt.set(qPrev)
        return False

select1Cnt.trace('w', validate_quality)
select1 = ttk.Combobox(subframe2, textvariable=select1Cnt)
select1["values"] = range(1, 101)
select1.set(50)
select1.pack(side=LEFT, pady=5)
select1["justify"] = RIGHT

subframe3 = ttk.Frame(root)
subframe3.pack(side=TOP, fill=X)
label3 = ttk.Label(subframe3, text="Lato blocco:")
label3.pack(side=LEFT, padx=5, pady=5)

block_size = StringVar()
label4textVar = StringVar()

prev = "1"
def update_block_size():
    global prev
    try:
        bs = int(block_size.get())
        if not bs > 0:
            raise ValueError
        prev = str(bs)
        label4textVar.set('x 8 = ' + str(bs * 8) + ' px')
    except ValueError:
        block_size.set(prev)
        return False
    return True

edit = ttk.Entry(subframe3, textvariable=block_size,
    validatecommand=update_block_size, validate='focusout')
edit.insert(0, "1")
edit["justify"] = RIGHT
edit["width"] = 4
edit.pack(side=LEFT, pady=5)

label4 = ttk.Label(subframe3)
label4['textvariable'] = label4textVar
label4textVar.set('x 8 = 8 px')
label4.pack(side=LEFT, pady=5)

subframe4 = ttk.Frame(root)
subframe4.pack(side=TOP, fill=BOTH, expand=YES)

f = pylab.figure()
ax1 = plt.subplot(1, 2, 1)
ax2 = plt.subplot(1, 2, 2, sharex=ax1, sharey=ax1)
canvas = FigureCanvasTkAgg(f, subframe4)
printed = False
canvas.get_tk_widget().update()
canvas._tkcanvas.update()
canvas.show()
plt.close(f)

def apply_compression():
    global printed
    if src_file == '':
        mbox.showerror('Nessuna immagine selezionata',
        'E\' necessario selezionare un\'immagine prima di procedere alla compressione!')
        return
    elif not os.path.isfile(src_file):
        mbox.showerror('Immagine selezionata non valida',
        'L\'immagine selezionata non è un file valido per la compressione!')
        return
    img = imread(src_file, True)
    src_res = img.shape
    bs = int(block_size.get()) * 8
    img = dct.complete_img(img, bs)
    f.texts = []
    f.suptitle((u'Risoluzione immagine %d x %d px' + \
        u' - Risoluzione immagine completata: %d x %d px' + \
        u"\nQualità: %s - Dimensione blocchi: %d x %d px - Numero blocchi: %d") \
        % (src_res[1], src_res[0], img.shape[1], img.shape[0],
        qPrev, bs, bs,
        (img.shape[1] * img.shape[0]) / (bs * bs)))
    ax1.clear()
    ax1.imshow(img, cmap='Greys_r')
    ax1.set_title('immagine sorgente')
    ax1.set_adjustable('box-forced')
    shk = img.copy()
    shk = dct.compress_img(shk, int(select1Cnt.get()), bs)
    ax2.clear()
    ax2.imshow(shk, cmap='Greys_r')
    ax2.set_title('immagine compressa')
    ax2.set_adjustable('box-forced')
    if not printed:
        canvas.get_tk_widget().pack(side=TOP, fill=BOTH, expand=YES)
        canvas._tkcanvas.pack(fill=BOTH, expand=YES)
        toolbar = NavigationToolbar2TkAgg(canvas, subframe4)
        toolbar.pack(side=BOTTOM, fill=X)
        toolbar.update()
        printed = True
    canvas.draw()

button1 = ttk.Button(subframe1, text="Applica compressione", command=apply_compression)
button1.pack(side=LEFT, padx=5, pady=5)

root.mainloop()
