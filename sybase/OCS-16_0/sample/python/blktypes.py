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
# 	This example program demonstrates the use of different Python
#	object types as values in a bulk operation.
#	It demonstrates 
#	- the use of default values
#	- passing NULL values
#	- passing objects of different Python types.
#      
# Inputs
# ------         
#    	None.
#      
# 

import sybpydb
import sys
from sybpydb import Default as Default
from sybpydb import Date as Date
from sybpydb import Time as Time
from sybpydb import Timestamp as Timestamp
from decimal import Decimal as Decimal

conn = sybpydb.connect(dsn="user=sa;bulklogin=true;chainxacts=off")
cur = conn.cursor()
cur.execute("if exists (select name from sysdatabases where name = \"mydb\") \
		drop database mydb")
cur.execute("create database mydb")
cur.execute("exec sp_dboption mydb, \"select into/bulkcopy\", true")
cur.execute("use mydb")
cur.execute("checkpoint")


# Create a table
cur.execute("""create table mytable (
			s varchar(10) default 'xyz' null,
			i int default 11 null, 
			f float default 1.2 null,
			d decimal (4, 2) default 9.23 null,
			ti time default '11:59:59' null,
			da date default 'Jan 1, 2012' null,
			ts datetime default 'Feb 3, 2012 3:2:25' null,
			b varbinary(10) default '54321' null)
            """)
cur.close()


# Bulk copy in different Python types
blk = conn.blkcursor()
blk.copy("mytable", direction="in")

# Specify default values to be used during the bulk copy operation
blk.rowxfer([Default, Default, Default, Default, Default, Default, Default, Default])

# Specify different Python types as column values
s = 'abc'
i = 10
f = float(1.3)
d = Decimal("2.68")
ti = Time(12, 30, 15)
da = Date(1999, 6, 9)
ts = Timestamp(1999, 4, 5, 12, 30, 55)
b = bytearray(b'01234')
blk.rowxfer([s, i, f, d, ti, da, ts, b])

# Specify null values as column values
blk.rowxfer([None, None, None, None, None, None, None, None])

blk.done()


# Bulk copy out the rows
blk.copy("mytable", direction="out")
fmt = "%4s %5s %6s %8s %9s %11s %20s"
for row in blk:
        print("String: %s" % row[0]) 
        print("Integer: %s" % row[1]) 
        print("Float: %s" % row[2]) 
        print("Decimal: %s" % row[3]) 
        print("Time: %s" % row[4]) 
        print("Date: %s" % row[5]) 
        print("Datetime: %s" % row[6]) 
        if (row[7] == None):
                print("Bytearray: None") 
        else:
                print("Bytearray: %s" % row[7].decode()) 
        print("")

blk.done()


blk.close()
conn.close()
