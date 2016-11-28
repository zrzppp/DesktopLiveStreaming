
#include "mpegts.h"


static u_char mpegts_header[] = {

	/* TS */
	0x47, 0x40, 0x00, 0x10, 0x00,
	/* PSI */
	0x00, 0xb0, 0x0d, 0x00, 0x01, 0xc1, 0x00, 0x00,
	/* PAT */
	0x00, 0x01, 0xf0, 0x01,
	/* CRC */
	0x2e, 0x70, 0x19, 0x05,
	/* stuffing 167 bytes */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,

	/* TS */
	0x47, 0x50, 0x01, 0x10, 0x00,
	/* PSI */
	0x02, 0xb0, 0x17, 0x00, 0x01, 0xc1, 0x00, 0x00,
	/* PMT */
	0xe1, 0x00,
	0xf0, 0x00,
	0x1b, 0xe1, 0x00, 0xf0, 0x00, /* h264 */
	0x0f, 0xe1, 0x01, 0xf0, 0x00, /* aac */
	/*0x03, 0xe1, 0x01, 0xf0, 0x00,*/ /* mp3 */
	/* CRC */
	0x2f, 0x44, 0xb9, 0x9b, /* crc for aac */
	/*0x4e, 0x59, 0x3d, 0x1e,*/ /* crc for mp3 */
	/* stuffing 157 bytes */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};


/* 700 ms PCR delay */
#define HLS_DELAY  63000


static mpegts_int_t
mpegts_write_file(mpegts_file_t *file, u_char *in,
	size_t in_size)
{

	size_t   rc;


	rc = fwrite(in, in_size, 1, file->fd);
	if (rc < 0) {
		return MPEGTS_ERROR;
	}
	file->size += in_size;
	return MPEGTS_OK;

}


static mpegts_int_t
mpegts_write_header(mpegts_file_t *file)
{
	return mpegts_write_file(file, mpegts_header,
		sizeof(mpegts_header));
}


static u_char *
mpegts_write_pcr(u_char *p, uint64_t pcr)
{
	*p++ = (u_char)(pcr >> 25);
	*p++ = (u_char)(pcr >> 17);
	*p++ = (u_char)(pcr >> 9);
	*p++ = (u_char)(pcr >> 1);
	*p++ = (u_char)(pcr << 7 | 0x7e);
	*p++ = 0;

	return p;
}


static u_char *
mpegts_write_pts(u_char *p, mpegts_uint_t fb, uint64_t pts)
{
	mpegts_uint_t val;

	val = fb << 4 | (((pts >> 30) & 0x07) << 1) | 1;
	*p++ = (u_char)val;

	val = (((pts >> 15) & 0x7fff) << 1) | 1;
	*p++ = (u_char)(val >> 8);
	*p++ = (u_char)val;

	val = (((pts) & 0x7fff) << 1) | 1;
	*p++ = (u_char)(val >> 8);
	*p++ = (u_char)val;

	return p;
}


mpegts_int_t
mpegts_write_frame(mpegts_file_t *file,
	mpegts_frame_t *f, mpegts_buf_t *b)
{
	mpegts_uint_t  pes_size, header_size, body_size, in_size, stuff_size, flags;
	u_char      packet[188], *p, *base;
	mpegts_int_t   first, rc;

	first = 1;

	while (b->pos < b->last) {
		p = packet;

		f->cc++;

		*p++ = 0x47;
		*p++ = (u_char)(f->pid >> 8);

		if (first) {
			p[-1] |= 0x40;
		}

		*p++ = (u_char)f->pid;
		*p++ = 0x10 | (f->cc & 0x0f); /* payload */

		if (first) {

			if (f->key) {
				packet[3] |= 0x20; /* adaptation */

				*p++ = 7;    /* size */
				*p++ = 0x50; /* random access + PCR */

				p = mpegts_write_pcr(p, f->dts - HLS_DELAY);
			}

			/* PES header */

			*p++ = 0x00;
			*p++ = 0x00;
			*p++ = 0x01;
			*p++ = (u_char)f->sid;

			header_size = 5;
			flags = 0x80; /* PTS */

			if (f->dts != f->pts) {
				header_size += 5;
				flags |= 0x40; /* DTS */
			}

			pes_size = (b->last - b->pos) + header_size + 3;
			if (pes_size > 0xffff) {
				pes_size = 0;
			}

			*p++ = (u_char)(pes_size >> 8);
			*p++ = (u_char)pes_size;
			*p++ = 0x80; /* H222 */
			*p++ = (u_char)flags;
			*p++ = (u_char)header_size;

			p = mpegts_write_pts(p, flags >> 6, f->pts +
				HLS_DELAY);

			if (f->dts != f->pts) {
				p = mpegts_write_pts(p, 1, f->dts +
					HLS_DELAY);
			}

			first = 0;
		}

		body_size = (mpegts_uint_t)(packet + sizeof(packet) - p);
		in_size = (mpegts_uint_t)(b->last - b->pos);

		if (body_size <= in_size) {
			memcpy(p, b->pos, body_size);
			b->pos += body_size;

		}
		else {
			stuff_size = (body_size - in_size);

			if (packet[3] & 0x20) {

				/* has adaptation */

				base = &packet[5] + packet[4];
				p = memmove(base + stuff_size, base, p - base);
				memset(base, 0xff, stuff_size);
				packet[4] += (u_char)stuff_size;

			}
			else {

				/* no adaptation */

				packet[3] |= 0x20;
				p = memmove(&packet[4] + stuff_size, &packet[4],
					p - &packet[4]);

				packet[4] = (u_char)(stuff_size - 1);
				if (stuff_size >= 2) {
					packet[5] = 0;
					memset(&packet[6], 0xff, stuff_size - 2);
				}
			}

			memcpy(p, b->pos, in_size);
			b->pos = b->last;
		}

		rc = mpegts_write_file(file, packet, sizeof(packet));
		if (rc != MPEGTS_OK) {
			return rc;
		}
	}

	return MPEGTS_OK;
}




mpegts_int_t
mpegts_open_file(mpegts_file_t *file, char *path)
{

	file->fd = fopen(path, "wb");
	if (file->fd == NULL) {

		return MPEGTS_ERROR;
	}

	file->size = 0;

	if (mpegts_write_header(file) != MPEGTS_OK) {
		mpegts_close_file(file->fd);
		return MPEGTS_ERROR;
	}

	return MPEGTS_OK;
}


mpegts_int_t
mpegts_close_file(mpegts_file_t *file)
{
	fclose(file->fd);
	return MPEGTS_OK;
}
