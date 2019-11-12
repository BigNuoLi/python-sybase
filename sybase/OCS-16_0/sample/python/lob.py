#
# Copyright Notice and Disclaimer
# -------------------------------
#       (c) Copyright 2013.
#       SAP AG or an SAP affiliate company. All rights reserved.
#       Unpublished rights reserved under U.S. copyright laws.
#
#       SAP grants Licensee a non-exclusive license to use, reproduce,
#       modify, and distribute the sample source code below (the "Sample Code"),
#       subject to the following conditions:
#
#       (i) redistributions must retain the above copyright notice;
#
#       (ii) SAP shall have no obligation to correct errors or deliver
#       updates to the Sample Code or provide any other support for the
#       Sample Code;
#
#       (iii) Licensee may use the Sample Code to develop applications
#       (the "Licensee Applications") and may distribute the Sample Code in
#       whole or in part as part of such Licensee Applications, however in no
#       event shall Licensee distribute the Sample Code on a standalone basis;
#
#       (iv) and subject to the following disclaimer:
#       THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
#       INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
#       AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
#       SAP AG or an SAP affiliate company OR ITS LICENSORS BE LIABLE FOR ANY DIRECT, 
#	INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
#       BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
#       OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
#       ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
#       TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
#       USE OF THE SAMPLE CODE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
#       DAMAGE.
#
# Description
# -----------  
#    	This example demonstrates transferring lob data for text and
#	image columns. Data is first transferred from a bytearray object.
#	Next data is transferred from a file. Finally data is 
#	is retrieved from the lob columns and displayed.
#	Names of two files are required to run the sample. The files will
#	will be used as data for text and image columns.
#      
# Inputs
# ------         
#    	None.
#      
# Server Tables
# -------------  
#	None.
# 

import sybpydb
import os
import sys
import mmap

# Method to read data from a lob object
def getlobdata(lob):
    outarr = bytearray()
    chunk = bytearray(10)
    while True:
        len = lob.readinto(chunk);
        if (len == None):
             break
        outarr.extend(chunk[:len])
    print(outarr.decode())


# Get two file names
if len(sys.argv) < 3:
    sys.exit("Usage: %s file1 file2" % sys.argv[0])
if not os.path.exists(sys.argv[1]):
    sys.exit('ERROR: file %s not found!' % sys.argv[1])
if not os.path.exists(sys.argv[2]):
    sys.exit('ERROR: file %s not found!' % sys.argv[2])

conn = sybpydb.connect(user='sa', password='', dsn='chainxacts=off')
cur = conn.cursor()
cur.execute("if exists (select name from sysdatabases where name = \"mydb\") \
                drop database mydb")
cur.execute("create database mydb")
cur.execute("use mydb")
cur.execute("create table mytable (i int, t1 text, i1 image)")


# First transfer text and image data using a bytearray.
arr1 = bytearray(b"hello this is some text data")
lob1 = sybpydb.Lob(sybpydb.TEXT, arr1)
arr2 = bytearray(b"hello this is some image data")
lob2 = sybpydb.Lob(sybpydb.IMAGE, arr2)
cur.execute("insert into mytable values (?, ?, ?)", [1, lob1, lob2])

# Next transfer data from a file using memory maps.
# Create a temporary file.
fh1 = open(sys.argv[1], "rb")
mp1 = mmap.mmap(fh1.fileno(), 0 , access=mmap.ACCESS_READ)
arr1 = bytearray(mp1)
lob1 = sybpydb.Lob(sybpydb.TEXT, arr1)
# Create a temporary file.
fh2 = open(sys.argv[2], "rb")
mp2 = mmap.mmap(fh2.fileno(), 0 , access=mmap.ACCESS_READ)
arr2 = bytearray(mp2)
lob2 = sybpydb.Lob(sybpydb.IMAGE, arr2)
cur.execute("insert into mytable values (?, ?, ?)", [2, lob1, lob2])


cur.execute("select * from mytable")

# The rows should be fetched one by one.
# Fetch the first row and read the lob data one column at a time.
row = cur.fetchone()
print(row[0])
# Now read the lob data for the columns one by one
getlobdata(row[1])
getlobdata(row[2])
print("")

# Fetch the second row.
row = cur.fetchone()
print(row[0])
# Now read the lob data for the columns one by one
getlobdata(row[1])
getlobdata(row[2])

cur.close()
conn.close()
