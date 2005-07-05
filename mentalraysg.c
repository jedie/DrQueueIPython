// 
// Copyright (C) 2001,2002,2003,2004 Jorge Daza Garcia-Blanes
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA	 02111-1307
// USA
// 

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>

#include "mentalraysg.h"
#include "libdrqueue.h"

char *mentalraysg_create (struct mentalraysgi *info)
{
	/* This function creates the mentalray render script based on the information given */
	/* Returns a pointer to a string containing the path of the just created file */
	/* Returns NULL on failure and sets drerrno */
	FILE *f;
	FILE *etc_mentalray_sg;			/* The mentalray script generator configuration file */
	int fd_etc_mentalray_sg,fd_f;
	static char filename[BUFFERLEN];
	char fn_etc_mentalray_sg[BUFFERLEN]; /* File name pointing to DRQUEUE_ETC/mentalray.sg */
	char buf[BUFFERLEN];
	int size;
	char *p;			/* Scene filename without path */

	/* Check the parameters */
	if ((!strlen(info->renderdir)) || (!strlen(info->scene))) {
		drerrno = DRE_NOTCOMPLETE;
		return NULL;
	}

	p = strrchr(info->scene,'/');
	p = ( p ) ? p+1 : info->scene;
	snprintf(filename,BUFFERLEN-1,"%s/%s.%lX",info->scriptdir,p,(unsigned long int)time(NULL));

	if ((f = fopen (filename, "a")) == NULL) {
		if (errno == ENOENT) {
			/* If its because the directory does not exist we try creating it first */
			if (mkdir (info->scriptdir,0775) == -1) {
				drerrno = DRE_COULDNOTCREATE;
				return NULL;
			} else if ((f = fopen (filename, "a")) == NULL) {
				drerrno = DRE_COULDNOTCREATE;
				return NULL;
			}
		} else {
			drerrno = DRE_COULDNOTCREATE;
			return NULL;
		}
	}

	fchmod (fileno(f),0777);

	/* So now we have the file open and so we must write to it */
	fprintf(f,"#!/bin/tcsh\n\n");
	fprintf(f,"set DRQUEUE_RD=%s\n",info->renderdir);
	fprintf(f,"set DRQUEUE_SCENE=%s\n",info->scene);
	fprintf(f,"set RF_OWNER=%s\n",info->file_owner);
	if (strlen(info->format)) {
		fprintf(f,"set FFORMAT=%s\n",info->format);
	}
	if (info->res_x != -1) {
		fprintf(f,"set RESX=%i\n",info->res_x);
	}
	if (info->res_y != -1) {
		fprintf(f,"set RESY=%i\n",info->res_y);
	}
	if (strlen(info->camera)) {
		fprintf(f,"set CAMERA=%s\n",info->camera);
	}
	if (strlen(info->image)) {
		fprintf(f,"set DRQUEUE_IMAGE=%s\n",info->image);
	}

	snprintf(fn_etc_mentalray_sg,BUFFERLEN-1,"%s/mentalray.sg",getenv("DRQUEUE_ETC"));

	fflush (f);

	if ((etc_mentalray_sg = fopen (fn_etc_mentalray_sg,"r")) == NULL) {
		fprintf(f,"\necho -------------------------------------------------\n");
		fprintf(f,"echo ATTENTION ! There was a problem opening: %s\n",fn_etc_mentalray_sg);
		fprintf(f,"echo So the default configuration will be used\n");
		fprintf(f,"echo -------------------------------------------------\n");
		fprintf(f,"\n\n");
		fprintf(f,"cd \"$DRQUEUE_RD\"\n");
		fprintf(f,"set BLOCK=`expr $DRQUEUE_FRAME + $DRQUEUE_BLOCKSIZE - 1`\n");
		fprintf(f,"if ($BLOCK > $DRQUEUE_ENDFRAME) then\n");
		fprintf(f,"\tset BLOCK = $DRQUEUE_ENDFRAME;\n");
		fprintf(f,"endif\n\n");
		fprintf(f,"mentalrayrender $DRQUEUE_SCENE -render $DRQUEUE_FRAME $BLOCK\n\n");
	} else {
		fd_etc_mentalray_sg = fileno (etc_mentalray_sg);
		fd_f = fileno (f);
		while ((size = read (fd_etc_mentalray_sg,buf,BUFFERLEN)) != 0) {
			write (fd_f,buf,size);
		}
		fclose(etc_mentalray_sg);
	}

	fclose(f);

	return filename;
}


char *mentalraysg_default_script_path (void)
{
	static char buf[BUFFERLEN];
	char *p;

	if (!(p = getenv("DRQUEUE_TMP"))) {
		return ("/drqueue_tmp/not/set/");
	}
	
	if (p[strlen(p)-1] == '/')
		snprintf (buf,BUFFERLEN-1,"%s",p);
	else
		snprintf (buf,BUFFERLEN-1,"%s/",p);

	return buf;
}



