/*                                                                       
 * Copyright (c) 2003 Century Software, Inc.   All Rights Reserved.     
 *                                                                       
 * This file is part of the PIXIL Operating Environment                 
 *                                                                       
 * The use, copying and distribution of this file is governed by one    
 * of two licenses, the PIXIL Commercial License, or the GNU General    
 * Public License, version 2.                                           
 *                                                                       
 * Licensees holding a valid PIXIL Commercial License may use this file 
 * in accordance with the PIXIL Commercial License Agreement provided   
 * with the Software. Others are governed under the terms of the GNU   
 * General Public License version 2.                                    
 *                                                                       
 * This file may be distributed and/or modified under the terms of the  
 * GNU General Public License version 2 as published by the Free        
 * Software Foundation and appearing in the file LICENSE.GPL included   
 * in the packaging of this file.                                      
 *                                                                       
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING  
 * THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A            
 * PARTICULAR PURPOSE.                                                  
 *                                                                       
 * RESTRICTED RIGHTS LEGEND                                             
 *                                                                     
 * Use, duplication, or disclosure by the government is subject to      
 * restriction as set forth in paragraph (b)(3)(b) of the Rights in     
 * Technical Data and Computer Software clause in DAR 7-104.9(a).       
 *                                                                      
 * See http://www.pixil.org/gpl/ for GPL licensing       
 * information.                                                         
 *                                                                      
 * See http://www.pixil.org/license.html or              
 * email cetsales@centurysoftware.com for information about the PIXIL   
 * Commercial License Agreement, or if any conditions of this licensing 
 * are not clear to you.                                                
 */



/*
**
** Imported "Include" Files
**
*/
#include <malloc.h>
#include <stdio.h>
#include <string.h>

#include "bmptools.h"



/*
**
** Local Constant Definitions
**
*/



/*
**
** Local Enumeration Definitions
**
*/



/*
**
** Local Structure Definitions
**
*/



/*
**
** Local Variable Declarations
**
*/
static unsigned char bitmask[] = {
    0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
};



/*
**
**
**
*/
static int
readByte(FILE * fp, unsigned char *result)
{
    if (fread(result, 1, 1, fp) != 1)
	return (-1);
    return (0);
}



/*
**
**
**
*/
static int
readWord(FILE * fp, unsigned short *result)
{
    unsigned char b1, b2;

    if (fread(&b1, 1, 1, fp) != 1)
	return (-1);
    if (fread(&b2, 1, 1, fp) != 1)
	return (-1);

    *result = (((unsigned short) b2) << 8) + b1;
    return (0);
}



/*
**
**
**
*/
static int
readLong(FILE * fp, unsigned int *result)
{
    unsigned char b1, b2, b3, b4;

    if (fread(&b1, 1, 1, fp) != 1)
	return (-1);
    if (fread(&b2, 1, 1, fp) != 1)
	return (-1);
    if (fread(&b3, 1, 1, fp) != 1)
	return (-1);
    if (fread(&b4, 1, 1, fp) != 1)
	return (-1);

    *result = (((unsigned int) b4) << 24) +
	(((unsigned int) b3) << 16) + (((unsigned int) b2) << 8) + b1;
    return (0);
}



/*
**
**
**
*/
static void
getMonochromePixel(BmpHandle * handle,
		   unsigned int xpos, unsigned short ypos, BmpColor * result)
{
    BmpPaletteEntry pentry;
    int res, sw;
    unsigned char bit, byte, ref;

    // calculate the scanline width - adjust for pixel padding
    sw = handle->info.padded_width / 8;

    // adjust the ypos (origin is lower-left corner for BMP images)
    ypos = (handle->info.height - 1) - ypos;

    // recover the pixel
    ref = xpos / 8;
    byte = *(handle->image + ((ypos * sw) + ref));
    bit = xpos - (ref * 8);

    // recover the pixel value
    res = byte & bitmask[bit];
    if (res)
	pentry = *(handle->palette + 1);
    else
	pentry = *(handle->palette + 0);

    // save result and exit
    result->red = pentry.red;
    result->green = pentry.green;
    result->blue = pentry.blue;
}



/*
**
** This function will load the specfied image into memory.
**
** The "path" parameter is a pointer to the buffer that contains
** the files path and filename.
**
** If successful, a handle to the loaded image is returned. If an
** error occurs during function execution, NULL is returned.
**
*/
BmpHandle *
bmpLoadImage(char *path)
{
    FILE *fp;
    BmpHandle *handle;
    unsigned int count, result, size;

    // open the file
    if ((fp = fopen(path, "r")) == NULL)
	return (NULL);

    // allocate memory from the system
    handle = (BmpHandle *) malloc(sizeof(BmpHandle));
    if (handle == NULL) {
	fclose(fp);
	return (NULL);
    }
    // read the BmpHeader structure
    result = readByte(fp, &handle->header.signature[0]);
    result += readByte(fp, &handle->header.signature[1]);
    result += readLong(fp, &handle->header.filesize);
    result += readLong(fp, &handle->header.reserved);
    result += readLong(fp, &handle->header.raster_offset);
    if (result != 0) {
	free(handle);
	fclose(fp);
	return (NULL);
    }
    // make sure this is a valid BMP file
    if (strcmp(handle->header.signature, "BM") != 0) {
	free(handle);
	fclose(fp);
	return (NULL);
    }
    // read the BmpInfo structure
    result = readLong(fp, &handle->info.size);
    result += readLong(fp, &handle->info.width);
    result += readLong(fp, &handle->info.height);
    result += readWord(fp, &handle->info.planes);
    result += readWord(fp, &handle->info.bits_per_pixel);
    result += readLong(fp, &handle->info.compression);
    result += readLong(fp, &handle->info.image_size);
    result += readLong(fp, &handle->info.hres);
    result += readLong(fp, &handle->info.vres);
    result += readLong(fp, &handle->info.colors_used);
    result += readLong(fp, &handle->info.colors_important);
    if (result != 0) {
	free(handle);
	fclose(fp);
	return (NULL);
    }
    // calculate the padded width
    result = handle->info.width / 32;
    if ((result * 32) == handle->info.width)
	handle->info.padded_width = result * 32;
    else
	handle->info.padded_width = (result + 1) * 32;

    // read the color palette
    if (handle->info.bits_per_pixel <= 8) {
	// determine number of palette entries
	if (handle->info.bits_per_pixel == 1)
	    size = 2;
	else if (handle->info.bits_per_pixel == 4)
	    size = 16;
	else if (handle->info.bits_per_pixel == 8)
	    size = 256;
	else {
	    free(handle);
	    fclose(fp);
	    return (NULL);
	}

	// allocate memory from the system
	handle->palette =
	    (BmpPaletteEntry *) malloc(size * sizeof(BmpPaletteEntry));
	if (handle->palette == NULL) {
	    free(handle);
	    fclose(fp);
	    return (NULL);
	}
	// load the palette
	count = 0;
	do {
	    readByte(fp, &((handle->palette + count)->red));
	    readByte(fp, &((handle->palette + count)->green));
	    readByte(fp, &((handle->palette + count)->blue));
	    readByte(fp, &((handle->palette + count)->reserved));
	} while (++count < size);
    } else
	handle->palette = NULL;

    // determine the size of the image
    if (handle->info.bits_per_pixel == 1)
	size = (handle->info.padded_width * handle->info.height) / 8;
    else if (handle->info.bits_per_pixel == 4)
	size = (handle->info.width * handle->info.height) / 2;
    else if (handle->info.bits_per_pixel == 8)
	size = handle->info.width * handle->info.height;
    else if (handle->info.bits_per_pixel == 16)
	size = handle->info.width * handle->info.height * 2;
    else if (handle->info.bits_per_pixel == 24)
	size = handle->info.width * handle->info.height * 3;
    else {
	if (handle->palette)
	    free(handle->palette);
	free(handle);
	fclose(fp);
	return (NULL);
    }

    // allocate memory from the system
    handle->image = (unsigned char *) malloc(size);
    if (handle->image == NULL) {
	if (handle->palette)
	    free(handle->palette);
	free(handle);
	fclose(fp);
	return (NULL);
    }
    // read the image
    if (fread(handle->image, 1, size, fp) != size) {
	if (handle->palette)
	    free(handle->palette);
	free(handle->image);
	free(handle);
	fclose(fp);
	return (NULL);
    }
    // return result and exit with no errors
    return (handle);
}



/*
**
** This function will close the specified BMP image handle and free
** all associated memory.
**
** The "handle" parameter specifies an image loaded into memory
** using the "bmpLoadImage()" function.
**
** If successful, '0' is returned. If an error occurs during function
** execution, a non-zero value is returned that describes the error.
**
*/
int
bmpCloseImage(BmpHandle * handle)
{
    free(handle->image);
    if (handle->palette)
	free(handle->palette);
    free(handle);
    return (0);
}



/*
**
** This function will recover the specified palette entry.
**
** The "handle" parameter specifies an image loaded into memory
** using the "bmpLoadImage()" function. The "entry" parameter
** specifies which palette entry to recover. The "result" parameter
** is a pointer to the location where the palette entry color is
** stored.
**
** If successful, '0' is returned. If an error occurs during function
** execution, a non-zero value is returned that describes the error.
**
*/
int
bmpGetPaletteEntry(BmpHandle * handle, int entry, BmpColor * result)
{
    int size;

    // recover the palette size
    if (handle->info.bits_per_pixel == 1)
	size = 2;
    else if (handle->info.bits_per_pixel == 4)
	size = 16;
    else if (handle->info.bits_per_pixel == 8)
	size = 256;
    else
	return (-1);

    // make sure the palette entry is legal
    if (entry >= size)
	return (-2);

    // recover the result
    result->red = (handle->palette + entry)->red;
    result->green = (handle->palette + entry)->green;
    result->blue = (handle->palette + entry)->blue;

    // exit with no errors
    return (0);
}



/*
**
** This function will recover the pixel value at the specified
** location.
**
** The "handle" parameter specifies an image loaded into memory
** using the "bmpLoadImage()" function. The "xpos" and "ypos" 
** parameters specify the pixel position in the image. These 
** values are referenced from the upper-left corner of the image.
** The "result" parameter is a pointer to the location where the
** color will be stored.
**
** If successful, '0' is returned. If an error occurs during function
** execution, a non-zero value is returned that describes the error.
**
*/
int
bmpGetPixel(BmpHandle * handle, int xpos, int ypos, BmpColor * result)
{
    // verify the pixel position
    if ((xpos < 0) ||
	(xpos >= handle->info.width) ||
	(ypos < 0) || (ypos >= handle->info.height))
	return (-1);

    // monochrome (two-color) palette image
    if (handle->info.bits_per_pixel == 1)
	getMonochromePixel(handle, xpos, ypos, result);

    // 16 color palette image
    else if (handle->info.bits_per_pixel == 4)
	return (-2);

    // 256 color palette image
    else if (handle->info.bits_per_pixel == 8)
	return (-3);

    // 65,536 color image
    else if (handle->info.bits_per_pixel == 16)
	return (-4);

    // 16,777,216 color image
    else if (handle->info.bits_per_pixel == 24)
	return (-5);

    // unknown image format
    else
	return (-6);

    // exit with no errors
    return (0);
}
