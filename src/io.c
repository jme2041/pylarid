/*****************************************************************************/
/*                               _                  _       _                */
/*                              | |                (_)     | |               */
/*               _ __    _   _  | |   __ _   _ __   _    __| |               */
/*              | '_ \  | | | | | |  / _` | | '__| | |  / _` |               */
/*              | |_) | | |_| | | | | (_| | | |    | | | (_| |               */
/*              | .__/   \__, | |_|  \__,_| |_|    |_|  \__,_|               */
/*              | |       __/ |                                              */
/*              |_|      |___/                                               */
/*                                                                           */
/*                        Load Arrays of Imaging Data                        */
/*                                                                           */
/* Copyright (c) 2021, Jeffrey M. Engelmann                                  */
/*                                                                           */
/* pylarid is released under the revised (3-clause) BSD license.             */
/* For details, see LICENSE.txt                                              */
/*                                                                           */
/*****************************************************************************/

/* io.c: File I/O for the larid.Dset class */

#define NO_IMPORT_ARRAY
#include "larid.h"
#ifdef LARID_ZLIB
 #include <zlib.h>
#endif

/*****************************************************************************/

/* Byte swapping */

#if defined(HAVE__BUILTIN_BSWAP16)
 #define swap2 __builtin_bswap16
#elif defined(HAVE__BYTESWAP_USHORT)
 #define swap2 _byteswap_ushort
#else
static uint16_t
swap2(uint16_t const x)
{
    return ((x & 0xffu) << 8) | (x >> 8);
}
#endif

#if defined(HAVE__BUILTIN_BSWAP32)
 #define swap4 __builtin_bswap32
#elif defined(HAVE__BYTESWAP_ULONG)
 #define swap4 _byteswap_ulong
#else
static uint32_t
swap4(uint32_t const x)
{
    return  ((x & 0xffu) << 24) |
            ((x & 0xff00u) << 8) |
            ((x & 0xff0000u) >> 8) |
            ( x >> 24);
}
#endif

#if defined(HAVE__BUILTIN_BSWAP64)
 #define swap8 __builtin_bswap64
#elif defined(HAVE__BYTESWAP_UINT64)
 #define swap8 _byteswap_uint64
#else
static uint64_t
swap8(uint64_t const x)
{
    return  ((x & 0xffULL) << 56) |
            ((x & 0xff00ULL) << 40) |
            ((x & 0xff0000ULL) << 24) |
            ((x & 0xff000000ULL) << 8) |
            ((x & 0xff00000000ULL) >> 8) |
            ((x & 0xff0000000000ULL) >> 24) |
            ((x & 0xff000000000000ULL) >> 40) |
            ( x >> 56);
}
#endif

static float
swapf(float const x)
{
    union { float f; uint32_t dword; } a = { .f = x };
    a.dword = swap4(a.dword);
    return a.f;
}

static double
swapd(double const x)
{
    union { double d; uint64_t qword; } a = { .d = x };
    a.qword = swap8(a.qword);
    return a.d;
}

/* Byte swap a NIfTI-1.1 header */

static void
swap_nifti_1_header(nifti_1_header * const restrict p)
{
    assert(p);
    p->sizeof_hdr = swap4(p->sizeof_hdr);
    p->extents = swap4(p->extents);
    p->session_error = swap2(p->session_error);
    for(size_t i = 0; i < sizeof(p->dim)/sizeof(*(p->dim)); ++i)
    {
        p->dim[i] = swap2(p->dim[i]);
    }
    p->intent_p1 = swapf(p->intent_p1);
    p->intent_p2 = swapf(p->intent_p2);
    p->intent_p3 = swapf(p->intent_p3);
    p->intent_code = swap2(p->intent_code);
    p->datatype = swap2(p->datatype);
    p->bitpix = swap2(p->bitpix);
    p->slice_start = swap2(p->slice_start);
    for(size_t i = 0; i < sizeof(p->pixdim)/sizeof(*(p->pixdim)); ++i)
    {
        p->pixdim[i] = swapf(p->pixdim[i]);
    }
    p->vox_offset = swapf(p->vox_offset);
    p->scl_slope = swapf(p->scl_slope);
    p->scl_inter = swapf(p->scl_inter);
    p->slice_end = swap2(p->slice_end);
    p->cal_max = swapf(p->cal_max);
    p->cal_min = swapf(p->cal_min);
    p->slice_duration = swapf(p->slice_duration);
    p->toffset = swapf(p->toffset);
    p->glmax = swap4(p->glmax);
    p->glmin = swap4(p->glmin);
    p->qform_code = swap2(p->qform_code);
    p->sform_code = swap2(p->sform_code);
    p->quatern_b = swapf(p->quatern_b);
    p->quatern_c = swapf(p->quatern_c);
    p->quatern_d = swapf(p->quatern_d);
    p->qoffset_x = swapf(p->qoffset_x);
    p->qoffset_y = swapf(p->qoffset_y);
    p->qoffset_z = swapf(p->qoffset_z);
    for(size_t i = 0; i < sizeof(p->srow_x)/sizeof(*(p->srow_x)); ++i)
    {
        p->srow_x[i] = swapf(p->srow_x[i]);
        p->srow_y[i] = swapf(p->srow_y[i]);
        p->srow_z[i] = swapf(p->srow_z[i]);
    }
}

/* Byte swap a NIfTI-2 header */

static void
swap_nifti_2_header(nifti_2_header * const restrict p)
{
    assert(p);
    p->sizeof_hdr = swap4(p->sizeof_hdr);
    p->datatype = swap2(p->datatype);
    p->bitpix = swap2(p->bitpix);
    for(size_t i = 0; i < sizeof(p->dim)/sizeof(*(p->dim)); ++i)
    {
        p->dim[i] = swap8(p->dim[i]);
    }
    p->intent_p1 = swapd(p->intent_p1);
    p->intent_p2 = swapd(p->intent_p2);
    p->intent_p3 = swapd(p->intent_p3);
    for(size_t i = 0; i < sizeof(p->pixdim)/sizeof(*(p->pixdim)); ++i)
    {
        p->pixdim[i] = swapd(p->pixdim[i]);
    }
    p->vox_offset = swap8(p->vox_offset);
    p->scl_slope = swapd(p->scl_slope);
    p->scl_inter = swapd(p->scl_inter);
    p->cal_max = swapd(p->cal_max);
    p->cal_min = swapd(p->cal_min);
    p->slice_duration = swapd(p->slice_duration);
    p->toffset = swapd(p->toffset);
    p->slice_start = swap8(p->slice_start);
    p->slice_end = swap8(p->slice_end);
    p->qform_code = swap4(p->qform_code);
    p->sform_code = swap4(p->sform_code);
    p->quatern_b = swapd(p->quatern_b);
    p->quatern_c = swapd(p->quatern_c);
    p->quatern_d = swapd(p->quatern_d);
    p->qoffset_x = swapd(p->qoffset_x);
    p->qoffset_y = swapd(p->qoffset_y);
    p->qoffset_z = swapd(p->qoffset_z);
    for(size_t i = 0; i < sizeof(p->srow_x)/sizeof(*(p->srow_x)); ++i)
    {
        p->srow_x[i] = swapd(p->srow_x[i]);
        p->srow_y[i] = swapd(p->srow_y[i]);
        p->srow_z[i] = swapd(p->srow_z[i]);
    }
    p->slice_code = swap4(p->slice_code);
    p->xyzt_units = swap4(p->xyzt_units);
    p->intent_code =swap4(p->intent_code);
}

/*****************************************************************************/

/* Convert a NIfTI-1.1 header to a NIfTI-2 header */

static void
nifti_1to2(
    nifti_1_header const * const restrict p1,
    nifti_2_header * const restrict p2)
{
    assert(p1);
    assert(p2);

    p2->sizeof_hdr = sizeof(*p2);
    memcpy(p2->magic, p1->magic, 4);
    p2->magic[2] = 0x32;    /* Switch NIfTI version to 2 */
    p2->magic[4] = 0x0D;    /* Second four bytes match PNG magic number */
    p2->magic[5] = 0x0A;
    p2->magic[6] = 0x1A;
    p2->magic[7] = 0x0A;
    p2->datatype = p1->datatype;
    p2->bitpix = p1->bitpix;
    for(size_t i = 0; i < sizeof(p2->dim)/sizeof(*(p2->dim)); i++)
    {
        p2->dim[i] = (int64_t)p1->dim[i];
    }
    p2->intent_p1 = (double)p1->intent_p1;
    p2->intent_p2 = (double)p1->intent_p2;
    p2->intent_p3 = (double)p1->intent_p3;
    for(size_t i = 0; i < sizeof(p2->pixdim)/sizeof(*(p2->pixdim)); i++)
    {
        p2->pixdim[i] = (double)p1->pixdim[i];
    }
    p2->vox_offset = (int64_t)p1->vox_offset;   /* Keep NIfTI-1.1 offset */
    p2->scl_slope = (double)p1->scl_slope;
    p2->scl_inter = (double)p1->scl_inter;
    p2->cal_max = (double)p1->cal_max;
    p2->cal_min = (double)p1->cal_min;
    p2->slice_duration = (double)p1->slice_duration;
    p2->toffset = (double)p1->toffset;
    p2->slice_start = (int64_t)p1->slice_start;
    p2->slice_end = (int64_t)p1->slice_end;
    memcpy(p2->descrip, p1->descrip, sizeof(p2->descrip));
    memcpy(p2->aux_file, p1->aux_file, sizeof(p2->aux_file));
    p2->qform_code = (int32_t)p1->qform_code;
    p2->sform_code = (int32_t)p1->sform_code;
    p2->quatern_b = (double)p1->quatern_b;
    p2->quatern_c = (double)p1->quatern_c;
    p2->quatern_d = (double)p1->quatern_d;
    p2->qoffset_x = (double)p1->qoffset_x;
    p2->qoffset_y = (double)p1->qoffset_y;
    p2->qoffset_z = (double)p1->qoffset_z;
    for(size_t i = 0; i < sizeof(p2->srow_x)/sizeof(*(p2->srow_x)); i++)
    {
        p2->srow_x[i] = (double)p1->srow_x[i];
        p2->srow_y[i] = (double)p1->srow_y[i];
        p2->srow_z[i] = (double)p1->srow_z[i];
    }
    p2->slice_code = (int32_t)p1->slice_code;
    p2->xyzt_units = (int32_t)p1->xyzt_units;
    p2->intent_code = (int32_t)p1->intent_code;
    memcpy(p2->intent_name, p1->intent_name, sizeof(p2->intent_name));
    p2->dim_info = p1->dim_info;
}

/*****************************************************************************/

/* Convert a NIfTI-2 header to a NIfTI-1.1 header. Upon success, returns true.
 * Upon failure, sets a Python exception and returns false. This function sets
 * vox_offset and magic[1] to zero. These values depend on the output format
 * and are thus set by larid.Dset.to_nifti().
 */

static bool
nifti_2to1(
    nifti_2_header const * const restrict p2,
    nifti_1_header * const restrict p1)
{
    assert(!PyErr_Occurred());
    assert(p2);
    assert(p1);

    memset(p1, 0, sizeof(*p1));
    p1->sizeof_hdr = sizeof(*p1);
    p1->dim_info = p2->dim_info;
    for(size_t i = 0; i < sizeof(p1->dim)/sizeof(*(p1->dim)); i++)
    {
        if(p2->dim[i] < INT16_MIN || p2->dim[i] > INT16_MAX) goto range;
        p1->dim[i] = (int16_t)p2->dim[i];
    }
    p1->intent_p1 = (float)p2->intent_p1;
    p1->intent_p2 = (float)p2->intent_p2;
    p1->intent_p3 = (float)p2->intent_p3;
    if(p2->intent_code < INT16_MIN || p2->intent_code > INT16_MAX) goto range;
    p1->intent_code = (int16_t)p2->intent_code;
    p1->datatype = p2->datatype;
    p1->bitpix = p2->bitpix;
    if(p2->slice_start < INT16_MIN || p2->slice_start > INT16_MAX) goto range;
    p1->slice_start = (int16_t)p2->slice_start;
    for(size_t i = 0; i < sizeof(p1->pixdim)/sizeof(*(p1->pixdim)); i++)
    {
        p1->pixdim[i] = (float)p2->pixdim[i];
    }
    p1->scl_slope = (float)p2->scl_slope;
    p1->scl_inter = (float)p2->scl_inter;
    if(p2->slice_end < INT16_MIN || p2->slice_end > INT16_MAX) goto range;
    p1->slice_end = (int16_t)p2->slice_end;
    if(p2->slice_code < 0 || p2->slice_code > UINT8_MAX) goto range;
    p1->slice_code = (int8_t)p2->slice_code;
    if(p2->xyzt_units < 0 || p2->xyzt_units > UINT8_MAX) goto range;
    p1->xyzt_units = (int8_t)p2->xyzt_units;
    p1->cal_max = (float)p2->cal_max;
    p1->cal_min = (float)p2->cal_min;
    p1->slice_duration = (float)p2->slice_duration;
    p1->toffset = (float)p2->toffset;
    memcpy(p1->descrip, p2->descrip, sizeof(p1->descrip));
    memcpy(p1->aux_file, p2->aux_file, sizeof(p1->aux_file));
    if(p2->qform_code < INT16_MIN || p2->qform_code > INT16_MAX) goto range;
    p1->qform_code = (int16_t)p2->qform_code;
    if(p2->sform_code < INT16_MIN || p2->sform_code > INT16_MAX) goto range;
    p1->sform_code = (int16_t)p2->sform_code;
    p1->quatern_b = (float)p2->quatern_b;
    p1->quatern_c = (float)p2->quatern_c;
    p1->quatern_d = (float)p2->quatern_d;
    p1->qoffset_x = (float)p2->qoffset_x;
    p1->qoffset_y = (float)p2->qoffset_y;
    p1->qoffset_z = (float)p2->qoffset_z;
    for(size_t i = 0; i < sizeof(p1->srow_x)/sizeof(*(p1->srow_x)); i++)
    {
        p1->srow_x[i] = (float)p2->srow_x[i];
        p1->srow_y[i] = (float)p2->srow_y[i];
        p1->srow_z[i] = (float)p2->srow_z[i];
    }
    memcpy(p1->intent_name, p2->intent_name, sizeof(p1->intent_name));
    p1->magic[0] = 0x6E;
    p1->magic[2] = 0x31;

    assert(!PyErr_Occurred());
    return true;

range:
    PyErr_SetString(
        LaridError,
        "Could not export to NIfTI-1.1 format: header value out of range");
    return false;
}

/*****************************************************************************/

/* Test whether a string ends with a substring */

static bool
ends_with(char const * const restrict str, char const * const restrict substr)
{
    assert(str);
    assert(substr);

    size_t str_len = strlen(str);
    size_t substr_len = strlen(substr);
    return ((str_len >= substr_len) &&
            (strcmp(str + (str_len - substr_len), substr) == 0));
}

/*****************************************************************************/

/* Get the name of the image file that corresponds to a header file. Upon
 * success, returns a pointer to a char array containing the name of the image
 * file. This memory must be freed by the caller using PyMem_Free(). Upon
 * failure, sets a Python exception and returns NULL.
 */

static char *
get_image_fname(char const * const restrict header_fname)
{
    char * image_fname = NULL;

    assert(!PyErr_Occurred());
    assert(header_fname);

    size_t header_len = strlen(header_fname);

    image_fname = PyMem_Malloc(header_len + 1);
    if(!image_fname)
    {
        PyErr_SetNone(PyExc_MemoryError);
        goto except;
    }

    strcpy(image_fname, header_fname);

    /* The string substitution is safe because the length does not change.
     * hdr --> img or hdr.gz --> img.gz
     */
    if(ends_with(header_fname, ".hdr"))
    {
        strcpy(image_fname + (header_len - sizeof("hdr") + 1), "img");
    }
    else if(ends_with(header_fname, ".hdr.gz"))
    {
        strcpy(image_fname + (header_len - sizeof("hdr.gz") + 1), "img.gz");
    }
    else
    {
        Py_UNREACHABLE();
    }

    assert(!PyErr_Occurred());
    assert(image_fname);
    goto finally;

except:
    PyMem_Free(image_fname);
    assert(PyErr_Occurred());
    image_fname = NULL;

finally:
    return image_fname;
}

/*****************************************************************************/

/* These functions handle file I/O for both zlib and non-zlib builds of larid.
 * C standard library I/O functions are used on non-zlib builds and on zlib
 * builds when the user requests uncompressed output. zlib file I/O functions
 * are used on zlib builds when the user requests compressed output.
 *
 * These functions are based on "znzlib" by Mark Jenkinson (2004), which has
 * been released into the public domain.
 */

#ifdef LARID_ZLIB
 #define LARID_ZLIB_MAX_BLOCK_SIZE 0x40000000   /* (1 << 30) */
#endif

typedef struct larid_file_s {
    char * name;        /* File name */
    FILE * fp;          /* File pointer (non-zlib) */
#ifdef LARID_ZLIB
    gzFile zfp;         /* File pointer (zlib) */
#endif
} * LARID_FILE;

/* Open a file. If zip is true, use zlib functions on zlib builds. Upon
 * success, returns a LARID_FILE handle. Upon failure, sets a Python exception
 * and returns NULL.
 */

static LARID_FILE
larid_fopen(
    char const * const restrict path,
    char const * const restrict mode,
    bool const zip)
{
    (void)zip;      /* Suppress unused parameter warning on non-zlib builds */

    LARID_FILE ret = NULL;

    assert(!PyErr_Occurred());
    assert(path);
    assert(mode);

    /* Allocate space for the larid_file_s struct */
    ret = PyMem_Calloc(1, sizeof(struct larid_file_s));
    if(!ret)
    {
        PyErr_SetNone(PyExc_MemoryError);
        goto except;
    }

    /* Allocate space for the file name and copy it to the struct */
    ret->name = PyMem_Malloc(strlen(path) + 1);
    if(!ret->name)
    {
        PyErr_SetNone(PyExc_MemoryError);
        goto except;
    }
    strcpy(ret->name, path);

    /* Open the file */
#ifdef LARID_ZLIB
    if(zip)
    {
        /* Using compression */
        if((ret->zfp = gzopen(path, mode)) == NULL)
        {
            if(errno != 0)
            {
                PyErr_SetFromErrnoWithFilename(PyExc_IOError, ret->name);
            }
            else
            {
                PyErr_Format(PyExc_IOError,
                             "Error opening file: '%s'",
                             ret->name);
            }
            goto except;
        }
    }
    else
    {
#endif
        /* Not using compression */
        if((ret->fp = fopen(path, mode)) == NULL)
        {
            if(errno != 0)
            {
                PyErr_SetFromErrnoWithFilename(PyExc_IOError, ret->name);
            }
            else
            {
                PyErr_Format(PyExc_IOError,
                             "Error opening file: '%s'",
                             ret->name);
            }
            goto except;
        }
#ifdef LARID_ZLIB
    }
#endif

#ifdef LARID_ZLIB
    assert((ret->fp || ret->zfp) && !(ret->fp && ret->zfp));
#else
    assert(ret->fp);
#endif

    assert(!PyErr_Occurred());
    assert(ret);
    goto finally;

except:
    if(ret) PyMem_Free(ret->name);
    PyMem_Free(ret);
    assert(PyErr_Occurred());
    ret = NULL;

finally:
    return ret;
}

/* Close a file and free memory for f. If f is NULL, no action is taken. Upon
 * success, returns true. Upon failure, if an exception is not already set,
 * sets a Python exception and returns false. If an exception is already set,
 * returns false without setting a new exception.
 */

static bool
larid_fclose(LARID_FILE f)
{
    if(!f) return true;

    bool ret = false;
    assert(f->name);

#ifdef LARID_ZLIB
    if(f->zfp) ret = (gzclose(f->zfp) == Z_OK);
#endif
    if(f->fp) ret = (fclose(f->fp) == 0);

    if(!ret && !PyErr_Occurred())
    {
        if(errno != 0) PyErr_SetFromErrnoWithFilename(PyExc_IOError, f->name);
        else PyErr_Format(PyExc_IOError, "Error closing file: '%s'", f->name);
    }

    PyMem_Free(f->name);
    PyMem_Free(f);
    return ret;
}

/* Read from a file. Operate on the assumption that encountering EOF is
 * unexpected and therefore an error. This is a reasonable assumption for NIfTI
 * files which have a known header size and a data block size that can be
 * determined from the header. Upon success, returns true. Upon failure, sets a
 * Python exception and returns false.
 */

static bool
larid_fread(void * buf, size_t size, size_t count, LARID_FILE f)
{
    bool ret;

    assert(!PyErr_Occurred());
    assert(f);
    assert(f->name);

#ifdef LARID_ZLIB
    if(f->zfp)
    {
        char * cbuf = (char *)buf;
        size_t remain = size*count;
        unsigned n2read;
        int nread;

        while(remain > 0)
        {
            n2read = (remain < (size_t)LARID_ZLIB_MAX_BLOCK_SIZE ?
                remain : LARID_ZLIB_MAX_BLOCK_SIZE);
            nread = gzread(f->zfp, (void *)cbuf, n2read);
            if(nread != (int)n2read)
            {
                if(errno != 0)
                {
                    PyErr_SetFromErrnoWithFilename(PyExc_IOError, f->name);
                }
                else
                {
                    PyErr_Format(PyExc_IOError,
                                 "Error reading from file: '%s'",
                                 f->name);
                }
                goto except;
            }
            remain -= nread;
            cbuf += nread;
        }

        assert(remain == 0);
        assert(!PyErr_Occurred());
        ret = true;
        goto finally;
    }
#endif

    assert(f->fp);  /* If we get here on either build, f->fp must be valid */
    if(fread(buf, size, count, f->fp) != count)
    {
        if(errno != 0)
        {
            PyErr_SetFromErrnoWithFilename(PyExc_IOError, f->name);
        }
        else
        {
            PyErr_Format(PyExc_IOError,
                         "Error reading from file: '%s'",
                         f->name);
        }
        goto except;
    }

    assert(!PyErr_Occurred());
    ret = true;
    goto finally;

except:
    assert(PyErr_Occurred());
    ret = false;

finally:
    return ret;
}

/* Write to a file. Assume that short writes are an error. Upon success,
 * returns true. Upon failure, sets a Python exception and returns false.
 */

static bool
larid_fwrite(void const * buf, size_t size, size_t count, LARID_FILE f)
{
    bool ret;

    assert(!PyErr_Occurred());
    assert(f);
    assert(f->name);

#ifdef LARID_ZLIB
    if(f->zfp)
    {
        char const * cbuf = (char const *)buf;
        size_t remain = size*count;
        unsigned n2write;
        int nwritten;

        while(remain > 0)
        {
            n2write = (remain < (size_t)LARID_ZLIB_MAX_BLOCK_SIZE ?
                remain : LARID_ZLIB_MAX_BLOCK_SIZE);
            nwritten = gzwrite(f->zfp, (void const *)cbuf, n2write);
            if(nwritten != (int)n2write)
            {
                if(errno != 0)
                {
                    PyErr_SetFromErrnoWithFilename(PyExc_IOError, f->name);
                }
                else
                {
                    PyErr_Format(PyExc_IOError,
                                 "Error writing to file: '%s'",
                                 f->name);
                }
                goto except;
            }
            remain -= nwritten;
            cbuf += nwritten;
        }

        assert(remain == 0);
        assert(!PyErr_Occurred());
        ret = true;
        goto finally;
    }
#endif

    assert(f->fp);  /* If we get here on either build, f->fp must be valid */
    if(fwrite(buf, size, count, f->fp) != count)
    {
        if(errno != 0)
        {
            PyErr_SetFromErrnoWithFilename(PyExc_IOError, f->name);
        }
        else
        {
            PyErr_Format(PyExc_IOError,
                         "Error writing to file: '%s'",
                         f->name);
        }
        goto except;
    }

    assert(!PyErr_Occurred());
    ret = true;
    goto finally;

except:
    assert(PyErr_Occurred());
    ret = false;

finally:
    return ret;
}

/* Seek offset from the beginning of a file. Upon success, returns true. Upon
 * failure, sets a Python exception and returns false.
 */

static bool
larid_fseek(LARID_FILE f, long offset)
{
    bool ret;

    assert(!PyErr_Occurred());
    assert(f);
    assert(f->name);

#ifdef LARID_ZLIB
    if(f->zfp)
    {
        if(gzseek(f->zfp, offset, SEEK_SET) == -1)
        {
            if(errno != 0)
            {
                PyErr_SetFromErrnoWithFilename(PyExc_IOError, f->name);
            }
            else
            {
                PyErr_Format(PyExc_IOError,
                             "Error seeking file offset: '%s'",
                             f->name);
            }
            goto except;
        }

        assert(!PyErr_Occurred());
        ret = true;
        goto finally;
    }
#endif

    assert(f->fp);  /* If we get here on either build, f->fp must be valid */
    if(fseek(f->fp, offset, SEEK_SET) != 0)
    {
        if(errno != 0)
        {
            PyErr_SetFromErrnoWithFilename(PyExc_IOError, f->name);
        }
        else
        {
            PyErr_Format(PyExc_IOError,
                         "Error seeking file offset: '%s'",
                         f->name);
        }
        goto except;
    }

    assert(!PyErr_Occurred());
    ret = true;
    goto finally;

except:
    assert(PyErr_Occurred());
    ret = false;

finally:
    return ret;
}

/*****************************************************************************/

/* Internal function for reading from a NIfTI dataset. Upon success, returns a
 * new larid.Dset object. Upon failure, sets a Python exception and returns
 * NULL. To switch the data type from that found in the NIfTI file, provide a
 * nonzero datatype code to new_datatype. To switch the number of frames from
 * that found in the NIfTI file, provide a positive value to nt indicating the
 * requested number of frames. The read_data flag indicates whether to read the
 * data block from the NIfTI file (versus initializing the dataset's NumPy
 * array to all zeros). If new_datatype and/or new_nt are nonzero, read_data
 * must be false.
 */

static PyObject *
larid_read_nifti(
    PyTypeObject * const restrict type,
    char const * const restrict path,
    int16_t const new_datatype,
    int64_t const new_nt,
    bool const read_data)
{
    DsetObject * ret = NULL;
    LARID_FILE f1 = NULL;
    LARID_FILE f2 = NULL;
    bool st1, st2;

    assert(!PyErr_Occurred());
    assert(type);
    assert(path);
    assert(new_nt >= 0);

    /* Check that read_nifti is called appropriately from public functions */
    assert(!(new_datatype && read_data));   /* Cannot read with type change */
    assert(!(new_nt > 0 && read_data));     /* Cannot read with nt change */

    /* Check for a valid NIfTI extension */
    if( !ends_with(path, ".hdr") &&
        !ends_with(path, ".hdr.gz") &&
        !ends_with(path, ".nii") &&
        !ends_with(path, ".nii.gz"))
    {
        PyErr_Format(LaridError, "Invalid NIfTI file extension: '%s'", path);
        goto except;
    }

    /* Open the file */
    if(!(f1 = larid_fopen(path, "rb", true))) goto except;

    /* Read the first two bytes. If they are the gzip magic number (0x1f8b), it
     * means that the file could not be decompressed and therefore this build
     * of larid does not support gzipped files.
     */
    uint8_t buf[2];
    if(!larid_fread(buf, sizeof(*buf), sizeof(buf)/sizeof(*buf), f1))
        goto except;

    if(buf[0] == 0x1f && buf[1] == 0x8b)
    {
        PyErr_Format(LaridError,
                     "Gzipped files not supported: '%s'",
                     f1->name);
        goto except;
    }

    /* Return to the beginning of the file */
    if(!larid_fseek(f1, 0)) goto except;

    /* Read the first four bytes. This determines the NIfTI version and whether
     * byte swapping is necessary.
     */
    int32_t sizeof_hdr;
    if(!larid_fread(&sizeof_hdr, sizeof(sizeof_hdr), 1, f1)) goto except;

    uint32_t nifti_ver;
    bool needs_swap;
    if(sizeof_hdr == sizeof(nifti_1_header))
    {
        nifti_ver = 1;
        needs_swap = false;
    }
    else if(sizeof_hdr == sizeof(nifti_2_header))
    {
        nifti_ver = 2;
        needs_swap = false;
    }
    else
    {
        sizeof_hdr = swap4(sizeof_hdr);
        if(sizeof_hdr == sizeof(nifti_1_header))
        {
            nifti_ver = 1;
            needs_swap = true;
        }
        else if(sizeof_hdr == sizeof(nifti_2_header))
        {
            nifti_ver = 2;
            needs_swap = true;
        }
        else
        {
            PyErr_Format(LaridError,
                         "Invalid NIfTI header (sizeof_hdr): '%s'",
                         f1->name);
            goto except;
        }
    }

    /* Return to the beginning of the file */
    if(!larid_fseek(f1, 0)) goto except;

    /* Create a new Dset object */
    ret = (DsetObject *)Dset___new__(type, NULL, NULL);
    if(!ret) goto except;

    /* Coming in from NIfTI, datasets are in tkji order */
    ret->morder = LARID_MORDER_TKJI;

    /* Read the NIfTI header */
    if(nifti_ver == 1)
    {
        /* Read the NIfTI-1.1 header */
        nifti_1_header hdr1;
        if(!larid_fread(&hdr1, sizeof(hdr1), 1, f1)) goto except;

        /* Check the NIfTI-1.1 "magic" number */
        if(NIFTI_VERSION(hdr1) != 1)
        {
            PyErr_Format(LaridError,
                         "Invalid NIfTI header (NIfTI-1.1 magic number): '%s'",
                         f1->name);
            goto except;
        }

        /* Byte swap the NIfTI-1.1 header, if necessary */
        if(needs_swap) swap_nifti_1_header(&hdr1);

        /* Convert to NIfTI-2 */
        nifti_1to2(&hdr1, &ret->hdr);
        assert(NIFTI_VERSION(ret->hdr) == 2);
    }
    else
    {
        /* Read the NIfTI-2 header */
        if(!larid_fread(&ret->hdr, sizeof(ret->hdr), 1, f1)) goto except;

        /* Check the NIfTI-2 "magic" number */
        if( NIFTI_VERSION(ret->hdr) != 2 ||
            ret->hdr.magic[4] != 0x0D ||
            ret->hdr.magic[5] != 0x0A ||
            ret->hdr.magic[6] != 0x1A ||
            ret->hdr.magic[7] != 0x0A)
        {
            PyErr_Format(LaridError,
                         "Invalid NIfTI header (NIfTI-2 magic number): '%s'",
                         f1->name);
            goto except;
        }

        /* Byte swap the NIfTI-2 header, if necessary */
        if(needs_swap) swap_nifti_2_header(&ret->hdr);
    }

    /* Check that vox_offset is a multiple of 16 bytes (NIfTI standard) and
     * that it is less than LONG_MAX (because seek takes a long integer).
     */
    uint64_t vox_offset = ret->hdr.vox_offset;
    if(((uint32_t)vox_offset & (uint32_t)0xf) || vox_offset > LONG_MAX)
    {
        PyErr_Format(LaridError,
                     "Invalid NIfTI header (vox_offset): '%s'",
                     f1->name);
        goto except;
    }

    /* Check for the supported number of dimensions. Even if nt is changed
     * later, it must be supported by larid coming in from NIfTI.
     */
    if( ret->hdr.dim[0] < 3 ||
        ret->hdr.dim[0] > 5 ||
        (ret->hdr.dim[0] == 5 && ret->hdr.dim[4] > 1))
    {
        PyErr_Format(
            LaridError,
            "Only 3D, 4D, and statistical NIfTI files are supported: '%s'",
            f1->name);
        goto except;
    }

    /* Correct technically invalid but sometimes seen dims */
    if(ret->hdr.dim[0] == 3)
    {
        if(ret->hdr.dim[4] == 0) ret->hdr.dim[4] = 1;
        if(ret->hdr.dim[5] == 0) ret->hdr.dim[5] = 1;
    }
    else if(ret->hdr.dim[0] == 4)
    {
        if(ret->hdr.dim[5] == 0) ret->hdr.dim[5] = 1;
    }
    else if(ret->hdr.dim[0] == 5)
    {
        if(ret->hdr.dim[4] == 0) ret->hdr.dim[4] = 1;
    }
    ret->hdr.dim[6] = 1;        /* By convention, dim[6] is 1 */
    ret->hdr.dim[7] = 1;        /* By convention, dim[7] is 1 */

    /* Sanity check: dimensions cannot be less than 1 */
    if( DSET_NI(ret) < 1 ||
        DSET_NJ(ret) < 1 ||
        DSET_NK(ret) < 1 ||
        DSET_NT(ret) < 1)
    {
        PyErr_Format(
            LaridError,
            "Invalid NIfTI header (nonpositive dataset dimension): '%s'",
            f1->name);
        goto except;
    }

    /* Check for a supported datatype and get corresponding NumPy typenum. Even
     * if the datatype is changed later, it must be supported coming in from
     * NIfTI.
     */
    int typenum;
    if((typenum = datatype2typenum(ret->hdr.datatype)) == 0) goto except;

    /* Check that bitpix matches datatype */
    if(ret->hdr.bitpix != datatype2bitpix(ret->hdr.datatype))
    {
        PyErr_Format(
            LaridError,
            "Invalid NIfTI header (bitpix does not match datatype): '%s'",
            f1->name);
        goto except;
    }

    /* TODO: Check for a supported intent code and appropriate dims */

    /* Check pixdim */
    if( ret->hdr.pixdim[1] < LARID_ABS_TOL ||
        ret->hdr.pixdim[2] < LARID_ABS_TOL ||
        ret->hdr.pixdim[3] < LARID_ABS_TOL ||
        (ret->hdr.dim[4] > 1 && ret->hdr.pixdim[4] < LARID_ABS_TOL))
    {
        PyErr_Format(LaridError,
                     "Invalid NIfTI header (nonpositive pixdim): '%s'",
                     f1->name);
        goto except;
    }

    /* Correct technically invalid but sometimes seen pixdims */
    ret->hdr.pixdim[5] = 0.0;
    ret->hdr.pixdim[6] = 0.0;
    ret->hdr.pixdim[7] = 0.0;

    /* Change datatype (and bitpix) if requested */
    if(new_datatype != 0)
    {
        ret->hdr.datatype = new_datatype;
        ret->hdr.bitpix = datatype2bitpix(ret->hdr.datatype);
        typenum = datatype2typenum(ret->hdr.datatype);
        assert(typenum != 0);
    }

    /* Change nt if requested. This clears the intent information. */
    if(new_nt > 0)
    {
        ret->hdr.dim[0] = 4;
        ret->hdr.dim[4] = new_nt;
        ret->hdr.dim[5] = 1;
        ret->hdr.intent_code = NIFTI_INTENT_NONE;
        memset(ret->hdr.intent_name, 0, sizeof(ret->hdr.intent_name));
        ret->hdr.intent_p1 = 0.0;
        ret->hdr.intent_p2 = 0.0;
        ret->hdr.intent_p3 = 0.0;
        ret->hdr.toffset = 0.0;
        ret->hdr.xyzt_units &= 0x3;     /* Clear temporal units */
        if(new_nt == 1)
        {
            ret->hdr.pixdim[4] = 0.0;
        }
        else
        {
            ret->hdr.pixdim[4] = 1.0;
            ret->hdr.xyzt_units |= NIFTI_UNITS_SEC;
        }
    }

    /* Create a new NumPy array for the voxel data */
    npy_intp dim[4];
    DIM_TKJI(dim, DSET_NI(ret), DSET_NJ(ret), DSET_NK(ret), DSET_NT(ret));
    ret->data = PyArray_SimpleNew(4, dim, typenum); /* ret owns reference */
    if(!ret->data) goto except;

    /* If not reading data, zero the data block. Otherwise, read the data. */
    if(!read_data)
    {
        PyArray_FILLWBYTE((PyArrayObject *)ret->data, 0);
    }
    else
    {
        if(!NIFTI_ONEFILE(ret->hdr))
        {
            /* If the dataset isn't one file, open the image file */
            char * img = get_image_fname(path);
            if(!img) goto except;
            f2 = larid_fopen(img, "rb", true);
            PyMem_Free(img);
            if(!f2) goto except;
        }

        /* Seek to vox_offset */
        if(!larid_fseek(f2 ? f2 : f1, (long)vox_offset)) goto except;

        /* Read the data */
        if(!larid_fread(
            PyArray_DATA((PyArrayObject *)ret->data),
            PyArray_ITEMSIZE((PyArrayObject *)ret->data),
            PyArray_SIZE((PyArrayObject *)ret->data),
            f2 ? f2 : f1)) goto except;

        /* Byte swap, if needed */
        if(needs_swap)
        {
            if(!PyArray_Byteswap((PyArrayObject *)ret->data, NPY_TRUE))
                goto except;
        }
    }

    assert(!PyErr_Occurred());
    assert(ret);
    goto finally;

except:
    Py_XDECREF(ret);
    assert(PyErr_Occurred());
    ret = NULL;

finally:
    st1 = larid_fclose(f1);     /* Always returns true on second pass */
    f1 = NULL;
    st2 = larid_fclose(f2);     /* Always returns true on second pass */
    f2 = NULL;
    if(!st1 || !st2) goto except;

    return (PyObject *)ret;
}

/*****************************************************************************/

/* larid.Dset.like_nifti */

char const Dset_like_nifti__doc__[] = PyDoc_STR(
"like_nifti($type, /, path, datatype=None, nt=0)\n"
"--\n"
"\n"
"Create a new Dset object based on a NIfTI dataset, but with data\n"
"initialized to an array of zeros. The default datatype is the same\n"
"type that is stored in the NIfTI file. If the datatype argument is\n"
"provided, the Dset object has that datatype. The default number of\n"
"frames is the same as the number of frames in the NIfTI file. If the\n"
"nt argument is provided, the number of frames is set to nt. When\n"
"created, the data in the Dset object are in 'tkji' order.");

PyObject *
Dset_like_nifti(PyTypeObject * type, PyObject * args, PyObject * kwds)
{
    static char const * const kwlist[] = { "path", "datatype", "nt", NULL };

    PyObject * ret = NULL;
    PyObject * path = NULL;
    int16_t new_datatype = 0;   /* If datatype is not provided, remains 0 */
    int64_t new_nt = 0;         /* If nt is not provided, remains 0 */

    assert(!PyErr_Occurred());

    if(!PyArg_ParseTupleAndKeywords(args, kwds,
                                    "O&|O&L:Dset.like_nifti", (char **)kwlist,
                                    PyUnicode_FSConverter, &path,
                                    obj2datatype, &new_datatype,
                                    &new_nt)) goto except;

    /* Check new nt */
    if(new_nt < 0)
    {
        PyErr_SetString(PyExc_ValueError,
                        "Requested nt cannot be less than zero");
        goto except;
    }

    ret = larid_read_nifti(
        type,
        PyBytes_AsString(path),
        new_datatype,
        new_nt,
        false);

    if(!ret) goto except;

    assert(!PyErr_Occurred());
    assert(ret);
    goto finally;

except:
    Py_XDECREF(ret);
    assert(PyErr_Occurred());
    ret = NULL;

finally:
    Py_XDECREF(path);
    return ret;
}

/*****************************************************************************/

/* larid.Dset.from_nifti */

char const Dset_from_nifti__doc__[] = PyDoc_STR(
"from_nifti($type, /, path)\n"
"--\n"
"\n"
"Create a new Dset object from a NIfTI dataset. The voxel data from\n"
"the NIfTI dataset are read into the data array. When created, the\n"
"data in the Dset object are in 'tkji' order.");

PyObject *
Dset_from_nifti(PyTypeObject * type, PyObject * arg)
{
    PyObject * ret = NULL;
    PyObject * path = NULL;

    assert(!PyErr_Occurred());
    assert(arg);

    if(!PyUnicode_FSConverter(arg, &path)) goto except;

    ret = larid_read_nifti(
        type,
        PyBytes_AsString(path),
        0,
        0,
        true);

    if(!ret) goto except;

    assert(!PyErr_Occurred());
    assert(ret);
    goto finally;

except:
    Py_XDECREF(ret);
    assert(PyErr_Occurred());
    ret = NULL;

finally:
    Py_XDECREF(path);
    return ret;
}

/*****************************************************************************/

/* larid.Dset.to_nifti */

char const Dset_to_nifti__doc__[] = PyDoc_STR(
"to_nifti($self, /, path, nifti_ver=2)\n"
"--\n"
"\n"
"Write the dataset to a NIfTI file identified by path. If the file\n"
"already exists, it is overwritten. Use nifti_ver to specify the\n"
"NIfTI version (1 or 2).");

PyObject *
Dset_to_nifti(DsetObject * self, PyObject * args, PyObject * kwds)
{
    static char const * const kwlist[] = { "path", "nifti_ver", NULL };

    PyObject * ret = NULL;
    PyObject * path = NULL;
    LARID_FILE f1 = NULL;
    LARID_FILE f2 = NULL;
    int nifti_ver = 2;
    bool kjit = false, st1, st2;

    assert(!PyErr_Occurred());

    if(!PyArg_ParseTupleAndKeywords(args, kwds,
                                    "O&|i:Dset_to_nifti", (char **)kwlist,
                                    PyUnicode_FSConverter, &path,
                                    &nifti_ver)) goto except;

    /* Check nifti_ver */
    if(nifti_ver != 1 && nifti_ver != 2)
    {
        PyErr_Format(PyExc_ValueError, "Invalid nifti_ver: %d", nifti_ver);
        goto except;
    }

    /* Check the file extension to determine output format */
    bool nifti_pair;
    bool gzip;
    if(ends_with(PyBytes_AsString(path), ".hdr"))
    {
        nifti_pair = true;
        gzip = false;
    }
    else if(ends_with(PyBytes_AsString(path), ".hdr.gz"))
    {
        nifti_pair = true;
        gzip = true;
    }
    else if(ends_with(PyBytes_AsString(path), ".nii"))
    {
        nifti_pair = false;
        gzip = false;
    }
    else if(ends_with(PyBytes_AsString(path), ".nii.gz"))
    {
        nifti_pair = false;
        gzip = true;
    }
    else
    {
        PyErr_Format(LaridError, "Invalid NIfTI file extension: '%s'", path);
        goto except;
    }

#ifndef LARID_ZLIB
    if(gzip)
    {
        PyErr_Format(LaridError, "Gzipped files not supported: '%s'", path);
        goto except;
    }
#endif

    /* Open the file for writing */
    if(!(f1 = larid_fopen(PyBytes_AsString(path), "wb", gzip))) goto except;

    long vox_offset;
    if(nifti_ver == 1)
    {
        /* Create a NIfTI-1.1 header */
        nifti_1_header hdr1;
        if(!nifti_2to1(&self->hdr, &hdr1)) goto except;

        /* Set vox_offset and magic[1] */
        if(nifti_pair)
        {
            hdr1.vox_offset = 0.0f;
            hdr1.magic[1] = 0x69;           /* i */
        }
        else
        {
            hdr1.vox_offset = 352.0f;       /* no extensions */
            hdr1.magic[1] = 0x2B;           /* + */
        }

        vox_offset = (long)hdr1.vox_offset;

        /* Write the NIfTI header */
        if(!larid_fwrite(&hdr1, sizeof(hdr1), 1, f1)) goto except;
    }
    else
    {
        /* Set vox_offset and magic[1] */
        if(nifti_pair)
        {
            self->hdr.vox_offset = 0ll;
            self->hdr.magic[1] = 0x69;      /* i */
        }
        else
        {
            self->hdr.vox_offset = 544ll;   /* no extensions */
            self->hdr.magic[1] = 0x2B;      /* + */
        }

        vox_offset = (long)self->hdr.vox_offset;

        /* Write the NIfTI header */
        if(!larid_fwrite(&self->hdr, sizeof(self->hdr), 1, f1)) goto except;
    }

    if(nifti_pair)
    {
        /* Write four more bytes (this is typical for .hdr files) */
        int32_t zero = 0;
        if(!larid_fwrite(&zero, sizeof(zero), 1, f1)) goto except;

        /* Get the name of the corresponding image file and open it */
        char * img = get_image_fname(PyBytes_AsString(path));
        if(!img) goto except;
        f2 = larid_fopen(img, "wb", gzip);
        PyMem_Free(img);
        if(!f2) goto except;
    }

    /* Seek to vox_offset */
    if(!larid_fseek(f2 ? f2 : f1, vox_offset)) goto except;

    /* If needed, temporarily switch to tkji order */
    kjit = (self->morder == LARID_MORDER_KJIT);
    if(kjit)
    {
        PyObject * morder = PyUnicode_FromString("tkji");
        if(!morder) goto except;
        bool st = (Dset_set_morder(self, morder, NULL) == 0);
        Py_DECREF(morder);
        if(!st) goto except;
    }

    /* Write the data */
    if(!larid_fwrite(
        PyArray_DATA((PyArrayObject *)self->data),
        PyArray_ITEMSIZE((PyArrayObject *)self->data),
        PyArray_SIZE((PyArrayObject *)self->data),
        f2 ? f2 : f1)) goto except;

    assert(!PyErr_Occurred());
    Py_INCREF(Py_None);
    ret = Py_None;
    goto finally;

except:
    Py_XDECREF(ret);
    assert(PyErr_Occurred());
    ret = NULL;

finally:
    st1 = larid_fclose(f1);     /* Always returns true on subsequent passes */
    f1 = NULL;
    st2 = larid_fclose(f2);     /* Always returns true on subsequent passes */
    f2 = NULL;
    if(!st1 || !st2) goto except;

    if(kjit)
    {
        /* If needed, switch back to kjit order */
        PyObject * morder = PyUnicode_FromString("kjit");
        if(!morder) goto except;
        bool st = (Dset_set_morder(self, morder, NULL) == 0);
        Py_DECREF(morder);
        if(!st) goto except;
    }

    Py_XDECREF(path);           /* This is only called once */
    return ret;
}

/*****************************************************************************/
