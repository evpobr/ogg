/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggVorbis SOFTWARE CODEC SOURCE CODE.   *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE OggVorbis SOURCE CODE IS (C) COPYRIGHT 1994-2007             *
 * by the Xiph.Org Foundation http://www.xiph.org/                  *
 *                                                                  *
 ********************************************************************

 function: toplevel libogg include

 ********************************************************************/
#ifndef _OGG_H
#define _OGG_H

/**
 * @file ogg.h
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <ogg/os_types.h>

typedef struct {
  void *iov_base;
  size_t iov_len;
} ogg_iovec_t;

/** @brief The oggpack_buffer struct is used with libogg's bitpacking functions.
 *
 * You should never need to directly access anything in this structure.
 * @ingroup Bitpacking
 */
typedef struct {
  long endbyte;
  int  endbit;

  unsigned char *buffer;
  unsigned char *ptr;
  long storage;
} oggpack_buffer;

/** @brief This structure encapsulates data into one ogg bitstream page.
 *
 * Ogg pages are the fundamental unit of framing and interleave in an ogg
 * bitstream. They are made up of packet segments of 255 bytes each. There can
 * be as many as 255 packet segments per page, for a maximum page size of a
 * little under 64 kB. This is not a practical limitation as the segments can be
 * joined across page boundaries allowing packets of arbitrary size. In practice
 * many applications will not completely fill all pages because they flush the
 * accumulated packets periodically order to bound latency more tightly.
 *
 * For a complete description of ogg pages and headers, please refer to the
 * @ref md_doc_framing "framing" document.
 */
typedef struct {
  /** Pointer to the page header for this page.
   *
   * The exact contents of this header are defined in the framing spec document.
   */
  unsigned char *header;
  /** Length of the page header in bytes. */
  long header_len;
  /** Pointer to the data for this page. */
  unsigned char *body;
  /** Length of the body data in bytes. */
  long body_len;
} ogg_page;

/** @brief This structure contains current encode/decode data for a logical
 * bitstream.
 */
typedef struct {
  /** Pointer to data from packet bodies. */
  unsigned char   *body_data;
  /** Storage allocated for bodies in bytes (filled or unfilled). */
  long    body_storage;
  /** Amount of storage filled with stored packet bodies. */
  long    body_fill;
  /** Number of elements returned from storage. */
  long    body_returned;

  /** String of lacing values for the packet segments within the current page.
   * Each value is a byte, indicating packet segment length.
   */
  int     *lacing_vals;
  /** Pointer to the lacing values for the packet segments within the current
   * page.
   */
  ogg_int64_t *granule_vals;
  /** Total amount of storage (in bytes) allocated for storing lacing values. */
  long    lacing_storage;
  /** Fill marker for the current vs. total allocated storage of lacing values
   * for the page.
   */
  long    lacing_fill;
  /** Lacing value for current packet segment. */
  long    lacing_packet;
  /** Number of lacing values returned from lacing_storage. */
  long    lacing_returned;

  /** Temporary storage for page header during encode process, while the header
   * is being created.
   */
  unsigned char    header[282];
  /** Fill marker for header storage allocation. Used during the header creation
   * process.
   */
  int              header_fill;

  /** Marker set when the last packet of the logical bitstream has been
   * buffered.
   */
  int     e_o_s;
  /** Marker set after we have written the first page in the logical
   * bitstream. */
  int     b_o_s;
  /** Serial number of this logical bitstream. */
  long    serialno;
  /** Number of the current page within the stream. */
  long    pageno;
  /** Number of the current packet. */
  ogg_int64_t  packetno;
  /** Exact position of decoding/encoding process. */
  ogg_int64_t   granulepos;

} ogg_stream_state;

/** @brief This structure encapsulates the data and metadata for a single Ogg
 *  packet.
 *
 * The ogg_packet struct encapsulates the data for a single raw packet of data
 * and is used to transfer data between the ogg framing layer and the handling
 * codec.
 */
typedef struct {
  /** Pointer to the packet's data. This is treated as an opaque type by the ogg
   * layer.
   */
  unsigned char *packet;
  /** Indicates the size of the packet data in bytes. Packets can be of
   * arbitrary size.
   */
  long  bytes;
  /** Flag indicating whether this packet begins a logical bitstream.
   *
   * 1 indicates this is the first packet, 0 indicates any other position in the
   * stream.
   */
  long  b_o_s;
  /** Flag indicating whether this packet ends a bitstream.
   *
   * 1 indicates the last packet, 0 indicates any other position in the stream.
   */
  long  e_o_s;

  /** A number indicating the position of this packet in the decoded data.
   *
   * This is the last sample, frame or other unit of information ('granule')
   * that can be completely decoded from this packet.
   */
  ogg_int64_t  granulepos;

  /** Sequential number of this packet in the ogg bitstream. */
  ogg_int64_t  packetno;
} ogg_packet;

/** @brief Contains bitstream synchronization information.
 *
 * The ogg_sync_state struct tracks the synchronization of the current page.
 * It is used during decoding to track the status of data as it is read in,
 * synchronized, verified, and parsed into pages belonging to the various
 * logical bistreams in the current physical bitstream link.
 */
typedef struct {
  /** Pointer to buffered stream data. */
  unsigned char *data;
  /** Current allocated size of the stream buffer held in *data. */
  int storage;
  /** The number of valid bytes currently held in *data; functions as the buffer
   *  head pointer.
   */
  int fill;
  /** The number of bytes at the head of *data that have already been returned
   *  as pages; functions as the buffer tail pointer.
   */
  int returned;

  /** Synchronization state flag; nonzero if sync has not yet been attained or
   *  has been lost.
   */
  int unsynced;
  /** If synced, the number of bytes used by the synced page's header. */
  int headerbytes;
  /** If synced, the number of bytes used by the synced page's body. */
  int bodybytes;
} ogg_sync_state;

/** @defgroup Bitpacking Bitpacking
 * @brief Libogg contains a basic bitpacking library that is useful for
 * manipulating data within a buffer.
 * @{
 */

/** @brief Initializes a buffer for writing using this bitpacking library.
 *
 * This function initializes an oggpack_buffer for writing using the Ogg 
 * @ref Bitpacking "bitpacking" functions.
 * @param[in,out] b Buffer to be used for writing.
 * This is an ordinary data buffer with some extra markers to ease bit
 * navigation and manipulation.
 * @return No values are returned.
 */
extern void  oggpack_writeinit(oggpack_buffer *b);
/** Asynchronously checks error status of bitpacker write buffer.
 * This function checks the readiness status of an oggpack_buffer previously
 * initialized for writing using the Ogg bitpacking functions.
 * A write buffer that encounters an error (such as a failed malloc) will clear
 * its internal state and release any in-use memory, flagging itself as 'not
 * ready'. Subsequent attempts to write using the buffer will silently fail.
 * This error state may be detected at any later time by using
 * oggpack_writecheck(). It is safe but not necessary to call
 * oggpack_writeclear() on a buffer that has flagged an error and released its
 * resources.
 * @warning Important note to developers: Although libogg checks the results of
 * memory allocations, these checks are only useful on a narrow range of
 * embedded platforms. Allocation checks perform no useful service on a general
 * purpose desktop OS where pages are routinely overallocated and all
 * allocations succeed whether memory is available or not. The only way to
 * detect an out of memory condition on the vast majority of OSes is to watch
 * for and capture segmentation faults. This function is useful only to embedded
 * developers.
 * @param[in,out] b An oggpack_buffer previously initialized for writing.
 * @return
 * - zero: buffer is ready for writing
 * - nonzero: buffer is not ready or encountered an error
 */
extern int   oggpack_writecheck(oggpack_buffer *b);
extern void  oggpack_writetrunc(oggpack_buffer *b,long bits);
extern void  oggpack_writealign(oggpack_buffer *b);
extern void  oggpack_writecopy(oggpack_buffer *b,void *source,long bits);
extern void  oggpack_reset(oggpack_buffer *b);
extern void  oggpack_writeclear(oggpack_buffer *b);
extern void  oggpack_readinit(oggpack_buffer *b,unsigned char *buf,int bytes);
extern void  oggpack_write(oggpack_buffer *b,unsigned long value,int bits);
extern long  oggpack_look(oggpack_buffer *b,int bits);
extern long  oggpack_look1(oggpack_buffer *b);
extern void  oggpack_adv(oggpack_buffer *b,int bits);
extern void  oggpack_adv1(oggpack_buffer *b);
extern long  oggpack_read(oggpack_buffer *b,int bits);
extern long  oggpack_read1(oggpack_buffer *b);
extern long  oggpack_bytes(oggpack_buffer *b);
extern long  oggpack_bits(oggpack_buffer *b);
extern unsigned char *oggpack_get_buffer(oggpack_buffer *b);

/** @}*/

extern void  oggpackB_writeinit(oggpack_buffer *b);
extern int   oggpackB_writecheck(oggpack_buffer *b);
extern void  oggpackB_writetrunc(oggpack_buffer *b,long bits);
extern void  oggpackB_writealign(oggpack_buffer *b);
extern void  oggpackB_writecopy(oggpack_buffer *b,void *source,long bits);
extern void  oggpackB_reset(oggpack_buffer *b);
extern void  oggpackB_writeclear(oggpack_buffer *b);
extern void  oggpackB_readinit(oggpack_buffer *b,unsigned char *buf,int bytes);
extern void  oggpackB_write(oggpack_buffer *b,unsigned long value,int bits);
extern long  oggpackB_look(oggpack_buffer *b,int bits);
extern long  oggpackB_look1(oggpack_buffer *b);
extern void  oggpackB_adv(oggpack_buffer *b,int bits);
extern void  oggpackB_adv1(oggpack_buffer *b);
extern long  oggpackB_read(oggpack_buffer *b,int bits);
extern long  oggpackB_read1(oggpack_buffer *b);
extern long  oggpackB_bytes(oggpack_buffer *b);
extern long  oggpackB_bits(oggpack_buffer *b);
extern unsigned char *oggpackB_get_buffer(oggpack_buffer *b);

/* Ogg BITSTREAM PRIMITIVES: encoding **************************/

extern int      ogg_stream_packetin(ogg_stream_state *os, ogg_packet *op);
extern int      ogg_stream_iovecin(ogg_stream_state *os, ogg_iovec_t *iov,
                                   int count, long e_o_s, ogg_int64_t granulepos);
extern int      ogg_stream_pageout(ogg_stream_state *os, ogg_page *og);
extern int      ogg_stream_pageout_fill(ogg_stream_state *os, ogg_page *og, int nfill);
extern int      ogg_stream_flush(ogg_stream_state *os, ogg_page *og);
extern int      ogg_stream_flush_fill(ogg_stream_state *os, ogg_page *og, int nfill);

/* Ogg BITSTREAM PRIMITIVES: decoding **************************/

extern int      ogg_sync_init(ogg_sync_state *oy);
extern int      ogg_sync_clear(ogg_sync_state *oy);
extern int      ogg_sync_reset(ogg_sync_state *oy);
extern int      ogg_sync_destroy(ogg_sync_state *oy);
extern int      ogg_sync_check(ogg_sync_state *oy);

extern char    *ogg_sync_buffer(ogg_sync_state *oy, long size);
extern int      ogg_sync_wrote(ogg_sync_state *oy, long bytes);
extern long     ogg_sync_pageseek(ogg_sync_state *oy,ogg_page *og);
extern int      ogg_sync_pageout(ogg_sync_state *oy, ogg_page *og);
extern int      ogg_stream_pagein(ogg_stream_state *os, ogg_page *og);
extern int      ogg_stream_packetout(ogg_stream_state *os,ogg_packet *op);
extern int      ogg_stream_packetpeek(ogg_stream_state *os,ogg_packet *op);

/* Ogg BITSTREAM PRIMITIVES: general ***************************/

extern int      ogg_stream_init(ogg_stream_state *os,int serialno);
extern int      ogg_stream_clear(ogg_stream_state *os);
extern int      ogg_stream_reset(ogg_stream_state *os);
extern int      ogg_stream_reset_serialno(ogg_stream_state *os,int serialno);
extern int      ogg_stream_destroy(ogg_stream_state *os);
extern int      ogg_stream_check(ogg_stream_state *os);
extern int      ogg_stream_eos(ogg_stream_state *os);

extern void     ogg_page_checksum_set(ogg_page *og);

extern int      ogg_page_version(const ogg_page *og);
extern int      ogg_page_continued(const ogg_page *og);
extern int      ogg_page_bos(const ogg_page *og);
extern int      ogg_page_eos(const ogg_page *og);
extern ogg_int64_t  ogg_page_granulepos(const ogg_page *og);
extern int      ogg_page_serialno(const ogg_page *og);
extern long     ogg_page_pageno(const ogg_page *og);
extern int      ogg_page_packets(const ogg_page *og);

extern void     ogg_packet_clear(ogg_packet *op);


#ifdef __cplusplus
}
#endif

#endif  /* _OGG_H */
