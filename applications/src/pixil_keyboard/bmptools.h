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


#ifndef BMPTOOLS_H
#define BMPTOOLS_H


/*
**
** Imported "Include" Files
**
*/



/*
**
** Global Constant Definitions
**
*/



/*
**
** Global Enumeration Definitions
**
*/



/*
**
** Global Structure Definitions
**
*/
typedef struct
{
    char signature[2];
    unsigned int filesize;
    unsigned int reserved;
    unsigned int raster_offset;
}
BmpHeader;

typedef struct
{
    unsigned int size;
    unsigned int width;
    unsigned int padded_width;
    unsigned int height;
    unsigned short planes;
    unsigned short bits_per_pixel;
    unsigned int compression;
    unsigned int image_size;
    unsigned int hres;
    unsigned int vres;
    unsigned int colors_used;
    unsigned int colors_important;
}
BmpInfo;

typedef struct
{
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    unsigned char reserved;
}
BmpPaletteEntry;

typedef struct
{
    BmpHeader header;
    BmpInfo info;
    BmpPaletteEntry *palette;
    unsigned char *image;
}
BmpHandle;

typedef struct
{
    unsigned char red;
    unsigned char green;
    unsigned char blue;
}
BmpColor;



/*
**
** Global Variable Declarations
**
*/



/*
**
**  NAME: bmpLoadImage()
**
** USAGE: BmpHandle *bmpLoadImage(char *path);
**
** DESCR: This function will load the specfied image into memory.
**
** PARMS: The "path" parameter is a pointer to the buffer that contains
**        the files path and filename.
**
** RETRN: If successful, a handle to the loaded image is returned. If an
**        error occurs during function execution, NULL is returned.
**
*/
BmpHandle *bmpLoadImage(char *);



/*
**
**  NAME: bmpCloseImage()
**
** USAGE: int bmpCloseImage(BmpHandle *handle);
**
** DECSR: This function will close the specified BMP image handle and free
**        all associated memory.
**
** PARMS: The "handle" parameter specifies an image loaded into memory
**        using the "bmpLoadImage()" function.
**
** RETRN: If successful, '0' is returned. If an error occurs during function
**        execution, a non-zero value is returned that describes the error.
**
*/
int bmpCloseImage(BmpHandle *);



/*
**
**  NAME: bmpGetPaletteEntry()
**
** USAGE: int bmpGetPaletteEntry(BmpHandle *handle,
**                               int entry,
**                               BmpColor *result);
**
** DESCR: This function will recover the specified palette entry.
**
** PARMS: The "handle" parameter specifies an image loaded into memory
**        using the "bmpLoadImage()" function. The "entry" parameter
**        specifies which palette entry to recover. The "result" parameter
**        is a pointer to the location where the palette entry color is
**        stored.
**
** RETRN: If successful, '0' is returned. If an error occurs during function
**        execution, a non-zero value is returned that describes the error.
**
*/
int bmpGetPaletteEntry(BmpHandle *, int, BmpColor *);



/*
**
**  NAME: bmpGetPixel()
**
** USAGE: int bmpGetPixel(BmpHandle *handle,
**                        int xpos,
**                        int ypos,
**                        BmpColor *result);
**
** DESCR: This function will recover the pixel value at the specified
**        location.
**
** PARMS: The "handle" parameter specifies an image loaded into memory
**        using the "bmpLoadImage()" function. The "xpos" and "ypos" 
**        parameters specify the pixel position in the image. These 
**        values are referenced from the upper-left corner of the image.
**        The "result" parameter is a pointer to the location where the
**        color will be stored.
**
** RETRN: If successful, '0' is returned. If an error occurs during function
**        execution, a non-zero value is returned that describes the error.
**
*/
int bmpGetPixel(BmpHandle *, int, int, BmpColor *);

#endif /* BMPTOOLS_H */
