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
#	This example program demonstrates sending a RPC command
#	to a Server and the processing of row, parameter, and status
#	results returned from the remote procedure. It also demonstrates  
#	the use of different datatypes as parameters.
#       
#      
# Inputs
# ------         
# 	None.
#      
# Server Tables
# -------------  
#       None.
# 

import sybpydb

conn = sybpydb.connect(user='sa', password='')
cur = conn.cursor()

cur.execute("""
    create procedure pyproc
    @smallint1 smallint,
    @smallint2 smallint output,
    @int1 int,
    @int2 int output,
    @bigint1 bigint,
    @bigint2 bigint output,
    @real1 real,
    @real2 real output,
    @float1 float,
    @float2 float output,
    @time1 time,
    @time2 time output,
    @bigtime1 bigtime,
    @bigtime2 bigtime output,
    @char1 char(20) output
    as
    begin
	select @smallint2 = @smallint2 + @smallint1
	select @int2 = @int1 * @int1 
	select @bigint2 = @bigint1 + 5 
	select @real2 = @real1 + 1 
	select @float2 = @float1 - 1 
	select @time2 = dateadd(hh, 1, @time1) 
	select @bigtime2 = dateadd(us, 100000, @bigtime1) 
	select @char1 = "Character parameter"
    end
    """)

smallint_in = 100
smallint_out = sybpydb.OutParam(50)
int_in = 300
int_out = sybpydb.OutParam(int())
bigint_in = 9876543210000
bigint_out = sybpydb.OutParam(int())
real_in = 1.234567
real_out = sybpydb.OutParam(float())
float_in = 1.234567890123456
float_out = sybpydb.OutParam(float())
time_in = sybpydb.Time(9, 30, 10)
time_out = sybpydb.OutParam(sybpydb.Time())
bigtime_in = sybpydb.Time(2, 30, 15, 123456)
bigtime_out = sybpydb.OutParam(sybpydb.Time())
char_out = sybpydb.OutParam("")

vals = cur.callproc('pyproc', (smallint_in, smallint_out, int_in, int_out,
				bigint_in, bigint_out,
                                real_in, real_out, float_in, float_out, 
				time_in, time_out, bigtime_in, bigtime_out,
				char_out))

print ("Status = %d" % cur.proc_status)
print ("smallint = %d" % vals[1])
print ("int = %d" % vals[3])
print ("bigint = %ld" % vals[5])
print ("real = %f" % vals[7])
print ("float = %.15f" % vals[9])
print ("time = %s" % vals[11])
print ("bigtime = %s" % vals[13])
print ("char = %s" % vals[14])
cur.connection.commit()

# Remove the stored procedure
cur.execute("drop procedure pyproc")

cur.close()
conn.close()

