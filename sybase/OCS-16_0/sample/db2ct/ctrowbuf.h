/*
** Copyright Notice and Disclaimer
** -------------------------------
**      (c) Copyright 2013.
**      SAP AG or an SAP affiliate company. All rights reserved.
**      Unpublished rights reserved under U.S. copyright laws.
**
**      SAP grants Licensee a non-exclusive license to use, reproduce,
**      modify, and distribute the sample source code below (the "Sample Code"),
**      subject to the following conditions:
**
**      (i) redistributions must retain the above copyright notice;
**
**      (ii) SAP shall have no obligation to correct errors or deliver
**      updates to the Sample Code or provide any other support for the
**      Sample Code;
**
**      (iii) Licensee may use the Sample Code to develop applications
**      (the "Licensee Applications") and may distribute the Sample Code in
**      whole or in part as part of such Licensee Applications, however in no
**      event shall Licensee distribute the Sample Code on a standalone basis;
**
**      (iv) and subject to the following disclaimer:
**      THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
**      INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
**      AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
**      SAP AG or an SAP affiliate company OR ITS LICENSORS BE LIABLE FOR ANY DIRECT, 
**	INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
**	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
**	SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
**	CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
**	OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
**      USE OF THE SAMPLE CODE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
**      DAMAGE.
**
** Description
** -----------
** 	Client-Library program that uses array binding to buffer rows.
**
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctpublic.h>

/*
** Declare a COLUMN_DATA structure to hold column buffers for the data.
**
** Within each instance, value_array will be allocated as 
** a 2-d array:
**   CS_BYTE [ROW_BUF_SIZE][<datafmt.maxlength>]
**
** The length in bytes of the i'th value is lengths[i].
*/
typedef struct {
  CS_DATAFMT datafmt;      /* Format of the column */
  CS_INT     *lengths;     /* Lengths for each column value */
  CS_BYTE    *value_array; /* Array w/ one row to hold each column value */
} COLUMN_DATA;

/*
** Declare the structure to buffer rows.
*/
typedef struct { 
  CS_INT      magic;       /* Magic number for pointer validation. */
  CS_INT      ncols;       /* Number of columns */
  CS_INT      nrows;       /* Length of column arrays */
  CS_INT      nfetched;    /* Number of rows retrieved on last fetch */
  COLUMN_DATA *columns;    /* Column data areas */
} ROW_BUFFER;

/*
** Define a boolean macro that decides which datatypes this code 
** supports. If SUPPORTED_TYPE does not return non-zero for a 
** datatype, then columns of that type will be ignored.
**
** The idea here is that we don't want to allocate arrays for datatypes
** that can have big (> 255) values.
*/ 
#define SUPPORTED_TYPE(t) \
 ( ( t == CS_TEXT_TYPE \
     || t == CS_IMAGE_TYPE   \
    || t == CS_LONGCHAR_TYPE )  \
   ? 0 : 1 )

/*
** Prototypes.
*/ 
CS_RETCODE CS_PUBLIC rb_createbuf PROTOTYPE ((
	  CS_COMMAND     *cmd,
          CS_INT         nrows,
          CS_INT         *p_ncols,
          ROW_BUFFER     **p_rowbuf ));

CS_RETCODE CS_PUBLIC rb_dropbuf PROTOTYPE ((
          ROW_BUFFER     *rowbuf ));

CS_RETCODE CS_PUBLIC rb_fillbuf PROTOTYPE ((
	  CS_COMMAND     *cmd,
          ROW_BUFFER     *rowbuf,
          CS_INT         *n_fetched ));

CS_RETCODE CS_PUBLIC rb_describe PROTOTYPE ((
          ROW_BUFFER     *rowbuf,
          CS_INT         col,
          CS_DATAFMT     *p_datafmt ));

CS_INT CS_PUBLIC rb_datalen PROTOTYPE ((
          ROW_BUFFER     *rowbuf,
          CS_INT         row_num,
          CS_INT         col_num ));

CS_BYTE * CS_PUBLIC rb_dataptr PROTOTYPE ((
          ROW_BUFFER     *rowbuf,
          CS_INT         row_num,
          CS_INT         col_num ));

CS_RETCODE CS_PUBLIC rb_printrow PROTOTYPE ((
	  CS_COMMAND     *cmd,
          ROW_BUFFER     *rowbuf,
          CS_INT         row_num,
          FILE           *outfile ));



