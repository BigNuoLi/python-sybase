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
# 	This example program uses the bulk copy routines to copy
# 	data to a server table. The data is then copied out and displayed.
#      
# Inputs
# ------         
#    	None.
#      
# 

import sybpydb

data = [["first", 1], ["second", 2], ["third", 3], ["fourth", 4], ["fifth", 5]]

conn = sybpydb.connect(dsn="user=sa;bulklogin=true;chainxacts=off")
cur = conn.cursor()
cur.execute("if exists (select name from sysdatabases where name = \"mydb\") \
		drop database mydb")
cur.execute("create database mydb")
cur.execute("exec sp_dboption mydb, \"select into/bulkcopy\", true")
cur.execute("use mydb")
cur.execute("checkpoint")
cur.execute("create table mytable (name char(10), id int)")
cur.close()


# Bulk copy in the rows
blk = conn.blkcursor()
blk.copy("mytable", direction="in")
for row in data:
	blk.rowxfer(row)
blk.done()


# Bulk copy out the rows
print("Name       Id")
print("----       --")
blk.copy("mytable", direction="out")
while True:
	row = blk.rowxfer()
	if (not row):
		break
	print("%s %d" % (row[0], row[1]))
blk.done()


blk.close()
conn.close()
