/* Copyright 2012, Suzuki Plan.
 *----------------------------------------------------------------------------
 * [�ď�] VGE(Vide Game Engine) Version 1.00
 * [�T�v] Windows/Android�̃N���X�J�����������邽�߂̃G���W��. (QVGA��p)
 * [���l] �J�[�l����ʂɈˑ����Ȃ���������������(Windows/Android���p)
 *        �������AC�W���֐��͑S�ẴJ�[�l���Ŏg������̂Ƃ���.
 *----------------------------------------------------------------------------
 */
#ifdef _WIN32
#include <Windows.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "miniz.h"
#include "vge.h"
#include "vgeint.h"
#include "vgsdec.h"

/* �}�N����` */
#define abs(x) (x >= 0 ? (x) : -(x)) /* �ϐ��̐�Βl�𓾂� */
#define sgn(x) (x >= 0 ? (1) : (-1)) /* �ϐ��̕����𓾂�   */

/* �O���[�o���ϐ��̎��̐錾 */
struct _VRAM _vram;
struct _SLOT _slot[MAXSLOT];
struct _EFF _eff[MAXSLOT];
char* _note[MAXSLOT];
static uLong _notelen[MAXSLOT];
struct _TOUCH _touch;
unsigned char _mute;
unsigned char _pause;
int _tilt;

/* �����֐� */
static int gclip(unsigned char n, int* sx, int* sy, int* xs, int* ys, int* dx, int* dy);
static int gclip2(unsigned char n, int* sx, int* sy, int* xs, int* ys, int* dx, int* dy);
static void pixel(unsigned char* p, int x, int y, unsigned char c);
static void line(unsigned char* p, int fx, int fy, int tx, int ty, unsigned char c);
static void circle(unsigned char* p, int x, int y, int r, unsigned char c);
static void boxf(unsigned char* p, int fx, int fy, int tx, int ty, unsigned char c);

/*
 *----------------------------------------------------------------------------
 * [VGE-API] vge_gload: �O���t�B�b�N(�Ǝ��`��)���X���b�g�Ƀ��[�h����
 *----------------------------------------------------------------------------
 * ����:
 * - n [I] �X���b�g�ԍ�
 * - name [I] �t�@�C����
 *----------------------------------------------------------------------------
 * �߂�l: ������0�A���s���͔�0��Ԃ�.
 *----------------------------------------------------------------------------
 * ���:
 * - VRAM�̃p���b�g���́A���[�h�����O���t�B�b�N�̃p���b�g���ɏ㏑�������.
 * - �X���b�g�ԍ��Ƃ́A�摜�̕ۊǌɂ��Ӗ����鎯�ʎq�ł���.
 *   (vge_put ���Ăяo���ۂɗp����)
 * - �X���b�g�̏��́A�v���O������~����VGE�������I�ɊJ������.
 * - ���Ƀ��[�h�ς݂̃X���b�g�ɍă��[�h���s�����ꍇ�A�ȑO�̃X���b�g�̏��́A
 *   �����I�ɔj�������.
 * - �X���b�g�̊i�[�̈�̓q�[�v�̈�ł��邽�߁A���[�h��vge_init���ł̂ݍs����
 *   �𐄏�����.
 *----------------------------------------------------------------------------
 */
int vge_gload(unsigned char n, const char* name)
{
    unsigned char* bin;
    int rc = -1;
    int gsize;
    int size;

    /* �Â��X���b�g����j�� */
    if (_slot[n].dat) {
        free(_slot[n].dat);
        _slot[n].dat = NULL;
    }

    /* �����f�[�^�̎擾 */
    if (NULL == (bin = (unsigned char*)getbin(name, &size))) {
        goto ENDPROC;
    }

    if ('S' != bin[0] || 'Z' != bin[1]) {
        goto ENDPROC;
    }
    _slot[n].xs = bin[2] + 1;
    _slot[n].ys = bin[3] + 1;
    gsize = (_slot[n].xs) * (_slot[n].ys);

    /* �f�[�^�̈���m�� */
    if (NULL == (_slot[n].dat = (unsigned char*)malloc(gsize))) {
        goto ENDPROC;
    }

    /* �p���b�g����ǂݍ��� */
    bin += 4;
    memcpy(_vram.pal, bin, sizeof(_vram.pal));

    /* �摜�f�[�^��ǂݍ��� */
    bin += sizeof(_vram.pal);
    memcpy(_slot[n].dat, bin, gsize);

    /* �I������ */
    rc = 0;
ENDPROC:
    if (rc) {
        if (_slot[n].dat) {
            free(_slot[n].dat);
            _slot[n].dat = NULL;
        }
        _slot[n].xs = 0;
        _slot[n].ys = 0;
    }
    return rc;
}

/*
 *----------------------------------------------------------------------------
 * [VGE-API] vge_putBG: �X���b�g�f�[�^��BG�̈�֓]������
 *----------------------------------------------------------------------------
 * ����:
 * - n [I] �X���b�g�ԍ�
 * - sx [I] �X���b�g���X���W
 * - sy [I] �X���b�g���Y���W
 * - xs [I] ��
 * - ys [I] ����
 * - dx [I] VRAM��X���W
 * - dy [I] VRAM��Y���W
 *----------------------------------------------------------------------------
 * ���:
 * - BG�͈�x������������Ȃ�
 * - ���݂��Ȃ��X���b�g�ԍ����w�肵���ꍇ�A�����\�������Ƀ��^�[������
 * - �X���b�g�O�̗̈���w�肵���ꍇ�A�����\�������Ƀ��^�[������
 * - VRAM����͂ݏo�镔���̗̈�͕\�����Ȃ�
 *----------------------------------------------------------------------------
 */
void vge_putBG(unsigned char n, int sx, int sy, int xs, int ys, int dx, int dy)
{
    int i;
    int posT;
    int posF;
    if (gclip(n, &sx, &sy, &xs, &ys, &dx, &dy)) {
        return;
    }
    /* 1���C�����������R�s�[ */
    posT = dy * XSIZE + dx;
    posF = sy * _slot[n].xs + sx;
    for (i = 0; i < ys; i++) {
        memcpy(&(_vram.bg[(posT & 0x1ffff)]), &(_slot[n].dat[posF]), xs);
        posT += XSIZE;
        posF += _slot[n].xs;
    }
}

/*
 *----------------------------------------------------------------------------
 * [VGE-API] vge_putBG2: �p���b�g0��`�悵�Ȃ�vge_putBG
 * ����: vge_putBG�Ɠ���
 *----------------------------------------------------------------------------
 */
void vge_putBG2(unsigned char n, int sx, int sy, int xs, int ys, int dx, int dy)
{
    int i, j;
    int posT;
    int posF;
    if (gclip(n, &sx, &sy, &xs, &ys, &dx, &dy)) {
        return;
    }
    /* �����F�ȊO�̃s�N�Z����1�s�N�Z���Âݒ� */
    posT = dy * XSIZE + dx;
    posF = sy * _slot[n].xs + sx;
    for (j = 0; j < ys; j++) {
        for (i = 0; i < xs; i++) {
            if (_slot[n].dat[posF]) {
                _vram.bg[(posT & 0x1ffff)] = _slot[n].dat[posF];
            }
            posT++;
            posF++;
        }
        posT += XSIZE - xs;
        posF += _slot[n].xs - xs;
    }
}

/*
 *----------------------------------------------------------------------------
 * [VGE-API] vge_putSP: �X���b�g�f�[�^���X�v���C�g�̈�֓]������
 * ����: vge_putBG�Ɠ���
 * ���: �X�v���C�g�́A��x�\�������������d���̂悤�ȑ���
 *----------------------------------------------------------------------------
 */
void vge_putSP(unsigned char n, int sx, int sy, int xs, int ys, int dx, int dy)
{
    int i, j;
    int posT;
    int posF;
    if (gclip(n, &sx, &sy, &xs, &ys, &dx, &dy)) {
        return;
    }
    /* �����F�ȊO�̃s�N�Z����1�s�N�Z���Âݒ� */
    posT = dy * XSIZE + dx;
    posF = sy * _slot[n].xs + sx;
    for (j = 0; j < ys; j++) {
        for (i = 0; i < xs; i++) {
            if (_slot[n].dat[posF]) {
                _vram.sp[(posT & 0x1ffff)] = _slot[n].dat[posF];
            }
            posT++;
            posF++;
        }
        posT += XSIZE - xs;
        posF += _slot[n].xs - xs;
    }
}

/*
 *----------------------------------------------------------------------------
 * [VGE-API] vge_putSPX: �X���b�g�f�[�^���X�v���C�g�̈�֓]������(���蔻��t��)
 * ����: vge_putBG�Ɠ��� + (px,py)=�����蔻�������_�̈ʒu
 * ���: �X�v���C�g�́A��x�\�������������d���̂悤�ȑ���
 *----------------------------------------------------------------------------
 */
int vge_putSPX(unsigned char n, int sx, int sy, int xs, int ys, int dx, int dy, int px, int py)
{
    int ret = 0;
    int i, j;
    int posT;
    int posF;
    if (gclip(n, &sx, &sy, &xs, &ys, &dx, &dy)) {
        return 0;
    }
    /* �����F�ȊO�̃s�N�Z����1�s�N�Z���Âݒ� */
    posT = dy * XSIZE + dx;
    posF = sy * _slot[n].xs + sx;
    for (j = 0; j < ys; j++) {
        for (i = 0; i < xs; i++) {
            if (_slot[n].dat[posF]) {
                _vram.sp[(posT & 0x1ffff)] = _slot[n].dat[posF];
                if (dx + i == px && dy + j == py) ret = 1;
            }
            posT++;
            posF++;
        }
        posT += XSIZE - xs;
        posF += _slot[n].xs - xs;
    }
    return ret;
}

/*
 *----------------------------------------------------------------------------
 * [VGE-API] vge_putSPM: �X���b�g�f�[�^(mask)���X�v���C�g�̈�֓]������
 * ����: vge_putBG�Ɠ��� + mask�J���[(c)
 * ���: �X�v���C�g�́A��x�\�������������d���̂悤�ȑ���
 *----------------------------------------------------------------------------
 */
void vge_putSPM(unsigned char n, int sx, int sy, int xs, int ys, int dx, int dy, unsigned char c)
{
    int i, j;
    int posT;
    int posF;
    if (gclip(n, &sx, &sy, &xs, &ys, &dx, &dy)) {
        return;
    }
    /* �����F�ȊO�̃s�N�Z����1�s�N�Z���Âݒ� */
    posT = dy * XSIZE + dx;
    posF = sy * _slot[n].xs + sx;
    for (j = 0; j < ys; j++) {
        for (i = 0; i < xs; i++) {
            if (_slot[n].dat[posF]) {
                _vram.sp[(posT & 0x1ffff)] = c;
            }
            posT++;
            posF++;
        }
        posT += XSIZE - xs;
        posF += _slot[n].xs - xs;
    }
}

/*
 *----------------------------------------------------------------------------
 * [VGE-API] vge_putSP: �X���b�g�f�[�^���X�v���C�g�̈�֓]������
 * ����: vge_putBG�Ɠ���
 * ���: �X�v���C�g�́A��x�\�������������d���̂悤�ȑ���
 *----------------------------------------------------------------------------
 */
void vge_putSPH(unsigned char n, int sx, int sy, int xs, int ys, int dx, int dy)
{
    int i, j;
    int posT;
    int posF;
    xs /= 2;
    ys /= 2;
    if (gclip(n, &sx, &sy, &xs, &ys, &dx, &dy)) {
        return;
    }
    /* �����F�ȊO�̃s�N�Z����1�s�N�Z���Âݒ� */
    posT = dy * XSIZE + dx;
    posF = sy * _slot[n].xs + sx;
    for (j = 0; j < ys; j++) {
        for (i = 0; i < xs; i++) {
            if (_slot[n].dat[posF]) {
                _vram.sp[(posT & 0x1ffff)] = _slot[n].dat[posF];
            }
            posT++;
            posF += 2;
        }
        posT += XSIZE - xs;
        posF += (_slot[n].xs) * 2 - xs * 2;
    }
}

/*
 *----------------------------------------------------------------------------
 * [VGE-API] vge_putSPM: �X���b�g�f�[�^(mask)���X�v���C�g�̈�֓]������
 * ����: vge_putBG�Ɠ��� + mask�J���[(c)
 * ���: �X�v���C�g�́A��x�\�������������d���̂悤�ȑ���
 *----------------------------------------------------------------------------
 */
void vge_putSPMH(unsigned char n, int sx, int sy, int xs, int ys, int dx, int dy, unsigned char c)
{
    int i, j;
    int posT;
    int posF;
    xs /= 2;
    ys /= 2;
    if (gclip(n, &sx, &sy, &xs, &ys, &dx, &dy)) {
        return;
    }
    /* �����F�ȊO�̃s�N�Z����1�s�N�Z���Âݒ� */
    posT = dy * XSIZE + dx;
    posF = sy * _slot[n].xs + sx;
    for (j = 0; j < ys; j++) {
        for (i = 0; i < xs; i++) {
            if (_slot[n].dat[posF]) {
                _vram.sp[(posT & 0x1ffff)] = c;
            }
            posT++;
            posF += 2;
        }
        posT += XSIZE - xs;
        posF += (_slot[n].xs) * 2 - xs * 2;
    }
}

/*
 *----------------------------------------------------------------------------
 * [VGE-API] vge_putSP: �X���b�g�f�[�^���X�v���C�g�̈�֓]������
 * ����: vge_putBG�Ɠ���
 * ���: �X�v���C�g�́A��x�\�������������d���̂悤�ȑ���
 *----------------------------------------------------------------------------
 */
void vge_putSPD(unsigned char n, int sx, int sy, int xs, int ys, int dx, int dy)
{
    int i, j;
    int posT;
    int posF;
    if (gclip2(n, &sx, &sy, &xs, &ys, &dx, &dy)) {
        return;
    }
    /* �����F�ȊO�̃s�N�Z����1�s�N�Z���Âݒ� */
    posT = dy * XSIZE + dx;
    posF = sy * _slot[n].xs + sx;
    for (j = 0; j < ys; j++) {
        for (i = 0; i < xs; i++) {
            if (_slot[n].dat[posF]) {
                vge_pixelSP(dx + i * 2, dy + j * 2, _slot[n].dat[posF]);
                vge_pixelSP(dx + i * 2 + 1, dy + j * 2, _slot[n].dat[posF]);
                vge_pixelSP(dx + i * 2, dy + j * 2 + 1, _slot[n].dat[posF]);
                vge_pixelSP(dx + i * 2 + 1, dy + j * 2 + 1, _slot[n].dat[posF]);
            }
            posF++;
        }
        posF += _slot[n].xs - xs;
    }
}

/*
 *----------------------------------------------------------------------------
 * vge_putBG/vge_putSP���ʂ̃N���b�s���O����
 *----------------------------------------------------------------------------
 */
static int gclip(unsigned char n, int* sx, int* sy, int* xs, int* ys, int* dx, int* dy)
{
    /* �X���b�g�����[�h�ς݂��H */
    if (NULL == _slot[n].dat) {
        return -1;
    }
    /* �����ɂ݂͂����ĂȂ����H */
    if ((*sx) < 0 || _slot[n].xs < (*sx) + (*xs) || (*sy) < 0 || _slot[n].ys < (*sy) + (*ys) || (*dx) + (*xs) < 0 || XSIZE <= *dx || (*dy) + (*ys) < 0 ||
        YSIZE <= *dy) {
        return -1; /* �����݂͂����Y�� */
    }
    /* �����̃N���b�s���O����  */
    if ((*dx) < 0) {
        (*sx) -= (*dx);
        (*xs) += (*dx);
        (*dx) = 0;
    }
    /* �E���̃N���b�s���O����  */
    if (XSIZE < (*dx) + (*xs)) {
        (*xs) -= ((*dx) + (*xs)) - XSIZE;
    }
    /* �㑤�̃N���b�s���O����  */
    if ((*dy) < 0) {
        (*sy) -= (*dy);
        (*ys) += (*dy);
        (*dy) = 0;
    }
    /* �����̃N���b�s���O����  */
    if (YSIZE < (*dy) + (*ys)) {
        (*ys) -= ((*dy) + (*ys)) - YSIZE;
    }
    return 0;
}

/*
 *----------------------------------------------------------------------------
 * vge_putBG/vge_putSP���ʂ̃N���b�s���O����
 *----------------------------------------------------------------------------
 */
static int gclip2(unsigned char n, int* sx, int* sy, int* xs, int* ys, int* dx, int* dy)
{
    /* �X���b�g�����[�h�ς݂��H */
    if (NULL == _slot[n].dat) {
        return -1;
    }
    /* �����ɂ݂͂����ĂȂ����H */
    if ((*sx) < 0 || _slot[n].xs < (*sx) + (*xs) || (*sy) < 0 || _slot[n].ys < (*sy) + (*ys) || (*dx) + (*xs) * 2 < 0 || XSIZE <= *dx ||
        (*dy) + (*ys) * 2 < 0 || YSIZE <= *dy) {
        return -1; /* �����݂͂����Y�� */
    }
    return 0;
}

/*
 *----------------------------------------------------------------------------
 * [VGE-API] vge_clear: BG���w��p���b�g�ԍ��ŃN���A����
 *----------------------------------------------------------------------------
 * ����:
 * - c [I] �p���b�g�ԍ�
 *----------------------------------------------------------------------------
 */
void vge_clear(unsigned char c)
{
    memset(_vram.bg, c, sizeof(_vram.bg));
}

/*
 *----------------------------------------------------------------------------
 * [VGE-API] vge_scroll: BG���X�N���[��������
 *----------------------------------------------------------------------------
 * ����:
 * - x [I] X����(��=�E�X�N���[���A��=���X�N���[��)
 * - y [I] Y����(��=���X�N���[���A��=��X�N���[��)
 *----------------------------------------------------------------------------
 * ���:
 * - �X�N���[����̗̈�̓p���b�g0�ŃN���A
 *----------------------------------------------------------------------------
 */
void vge_scroll(int x, int y)
{
    int i;
    if (XSIZE <= x || YSIZE <= y || XSIZE <= (-x) || YSIZE <= (-y)) {
        vge_clear(0);
    }
    /* X���� */
    if (0 < x) {
        /* �E�X�N���[�� */
        for (i = 0; i < YSIZE; i++) {
            memmove(&_vram.bg[i * XSIZE + x], &_vram.bg[i * XSIZE], XSIZE - x);
            memset(&_vram.bg[i * XSIZE], 0, x);
        }
    } else if (x < 0) {
        /* ���X�N���[�� */
        for (i = 0; i < YSIZE; i++) {
            memmove(&_vram.bg[i * XSIZE], &_vram.bg[i * XSIZE - x], XSIZE + x);
            memset(&_vram.bg[i * XSIZE + (XSIZE + x)], 0, -x);
        }
    }
    /* Y���� */
    if (0 < y) {
        /* ���X�N���[�� */
        memmove(&_vram.bg[y * XSIZE], &_vram.bg[0], (YSIZE - y) * XSIZE);
        memset(&_vram.bg[0], 0, y * XSIZE);
    } else if (y < 0) {
        /* ��X�N���[�� */
        memmove(&_vram.bg[0], &_vram.bg[(-y) * XSIZE], (YSIZE + y) * XSIZE);
        memset(&_vram.bg[(YSIZE + y) * XSIZE], 0, (-y) * XSIZE);
    }
}

/*
 *----------------------------------------------------------------------------
 * [VGE-API] vge_pixelBG: BG�ʂɃs�N�Z����`�悷��
 *----------------------------------------------------------------------------
 * ����:
 * - x [I] X���W
 * - y [I] Y���W
 * - c [I] �p���b�g�ԍ�
 *----------------------------------------------------------------------------
 */
void vge_pixelBG(int x, int y, unsigned char c)
{
    pixel(_vram.bg, x, y, c);
}

/*
 *----------------------------------------------------------------------------
 * [VGE-API] vge_pixelSP: �X�v���C�g�ʂɃs�N�Z����`�悷��
 *----------------------------------------------------------------------------
 * ����:
 * - x [I] X���W
 * - y [I] Y���W
 * - c [I] �p���b�g�ԍ�
 *----------------------------------------------------------------------------
 */
void vge_pixelSP(int x, int y, unsigned char c)
{
    pixel(_vram.sp, x, y, c);
}

/*
 *----------------------------------------------------------------------------
 * vge_pixelBG, vge_pixelSP�̓�������
 *----------------------------------------------------------------------------
 */
static void pixel(unsigned char* p, int x, int y, unsigned char c)
{
    if (0 <= x && x < XSIZE && 0 <= y && y < YSIZE) {
        p[y * XSIZE + x] = c;
    }
}

/*
 *----------------------------------------------------------------------------
 * [VGE-API] vge_lineBG: BG�ʂɃ��C����`�悷��
 *----------------------------------------------------------------------------
 * ����:
 * - fx [I] X���W(��_)
 * - fy [I] Y���W(��_)
 * - tx [I] X���W(�I�_)
 * - ty [I] Y���W(�I�_)
 * - c [I] �p���b�g�ԍ�
 *----------------------------------------------------------------------------
 */
void vge_lineBG(int fx, int fy, int tx, int ty, unsigned char c)
{
    line(_vram.bg, fx, fy, tx, ty, c);
}

/*
 *----------------------------------------------------------------------------
 * [VGE-API] vge_lineBG: BG�ʂɃ��C����`�悷��
 *----------------------------------------------------------------------------
 * ����:
 * - fx [I] X���W(��_)
 * - fy [I] Y���W(��_)
 * - tx [I] X���W(�I�_)
 * - ty [I] Y���W(�I�_)
 * - c [I] �p���b�g�ԍ�
 *----------------------------------------------------------------------------
 */
void vge_lineSP(int fx, int fy, int tx, int ty, unsigned char c)
{
    line(_vram.sp, fx, fy, tx, ty, c);
}

/*
 *----------------------------------------------------------------------------
 * vge_lineBG��vge_lineSP�̋��ʏ���
 *----------------------------------------------------------------------------
 */
static void line(unsigned char* p, int fx, int fy, int tx, int ty, unsigned char c)
{
    int idx, idy;
    int ia, ib, ie;
    int w;
    idx = tx - fx;
    idy = ty - fy;
    if (!idx || !idy) {
        /* form -> to�ϊ� */
        if (tx < fx) {
            w = fx;
            fx = tx;
            tx = w;
        }
        if (ty < fy) {
            w = fy;
            fy = ty;
            ty = w;
        }
        if (0 == idy) {
            /* ������(Y���������Ȃ�) ... �ł������Ȑ��`�� */
            for (; fx <= tx; fx++) {
                pixel(p, fx, fy, c);
            }
        } else {
            /* ������(X���������Ȃ�) ... ��Ԗڂɍ����Ȑ��`�� */
            for (; fy <= ty; fy++) {
                pixel(p, fx, fy, c);
            }
        }
        return;
    }
    /* �ΐ�(DDA) */
    w = 1;
    ia = abs(idx);
    ib = abs(idy);
    if (ia >= ib) {
        ie = -abs(idy);
        while (w) {
            pixel(p, fx, fy, c);
            if (fx == tx) break;
            fx += sgn(idx);
            ie += 2 * ib;
            if (ie >= 0) {
                fy += sgn(idy);
                ie -= 2 * ia;
            }
        }
    } else {
        ie = -abs(idx);
        while (w) {
            pixel(p, fx, fy, c);
            if (fy == ty) break;
            fy += sgn(idy);
            ie += 2 * ia;
            if (ie >= 0) {
                fx += sgn(idx);
                ie -= 2 * ib;
            }
        }
    }
}

/*
 *----------------------------------------------------------------------------
 * [VGE-API] vge_circleBG: BG�ʂɃT�[�N����`�悷��
 *----------------------------------------------------------------------------
 * ����:
 * - x [I] X���W(��_)
 * - y [I] Y���W(��_)
 * - r [I] ���a
 * - c [I] �p���b�g�ԍ�
 *----------------------------------------------------------------------------
 */
void vge_circleBG(int x, int y, int r, unsigned char c)
{
    circle(_vram.bg, x, y, r, c);
}

/*
 *----------------------------------------------------------------------------
 * [VGE-API] vge_circleSP: �X�v���C�g�ʂɃT�[�N����`�悷��
 *----------------------------------------------------------------------------
 * ����:
 * - x [I] X���W(��_)
 * - y [I] Y���W(��_)
 * - r [I] ���a
 * - c [I] �p���b�g�ԍ�
 *----------------------------------------------------------------------------
 */
void vge_circleSP(int x, int y, int r, unsigned char c)
{
    circle(_vram.sp, x, y, r, c);
}

/*
 *----------------------------------------------------------------------------
 * vge_circleBG��vge_circleSP�̋��ʏ���
 *----------------------------------------------------------------------------
 */
static void circle(unsigned char* p, int x, int y, int r, unsigned char c)
{
    float x1, y1, x2, y2;
    int flg;

    flg = 0;
    x1 = (float)r;
    y1 = 0.0;

    while (!flg) {
        x2 = x1 - (y1 / 64.0f);
        y2 = y1 + (x2 / 64.0f);
        pixel(p, (int)x2 + x, (int)y2 + y, c);
        x1 = x2;
        y1 = y2;
        flg = (((x2 > r - 1.0f) && (x2 < r)) && ((y2 > -1.0f) && (y2 < 0.0f)));
    }
}

/*
 *----------------------------------------------------------------------------
 * [VGE-API] vge_boxBG: BG�ʂɃ{�b�N�X��`�悷��
 *----------------------------------------------------------------------------
 * ����:
 * - fx [I] X���W(��_)
 * - fy [I] Y���W(��_)
 * - tx [I] X���W(�I�_)
 * - ty [I] Y���W(�I�_)
 * - c [I] �p���b�g�ԍ�
 *----------------------------------------------------------------------------
 */
void vge_boxBG(int fx, int fy, int tx, int ty, unsigned char c)
{
    vge_lineBG(fx, fy, tx, fy, c);
    vge_lineBG(fx, ty, tx, ty, c);
    vge_lineBG(fx, fy, fx, ty, c);
    vge_lineBG(tx, fy, tx, ty, c);
}

/*
 *----------------------------------------------------------------------------
 * [VGE-API] vge_boxBG: �X�v���C�g�ʂɃ{�b�N�X��`�悷��
 *----------------------------------------------------------------------------
 * ����:
 * - fx [I] X���W(��_)
 * - fy [I] Y���W(��_)
 * - tx [I] X���W(�I�_)
 * - ty [I] Y���W(�I�_)
 * - c [I] �p���b�g�ԍ�
 *----------------------------------------------------------------------------
 */
void vge_boxSP(int fx, int fy, int tx, int ty, unsigned char c)
{
    vge_lineSP(fx, fy, tx, fy, c);
    vge_lineSP(fx, ty, tx, ty, c);
    vge_lineSP(fx, fy, fx, ty, c);
    vge_lineSP(tx, fy, tx, ty, c);
}

/*
 *----------------------------------------------------------------------------
 * [VGE-API] vge_boxBG: BG�ʂɓh��Ԃ��{�b�N�X��`�悷��
 *----------------------------------------------------------------------------
 * ����:
 * - fx [I] X���W(��_)
 * - fy [I] Y���W(��_)
 * - tx [I] X���W(�I�_)
 * - ty [I] Y���W(�I�_)
 * - c [I] �p���b�g�ԍ�
 *----------------------------------------------------------------------------
 */
void vge_boxfBG(int fx, int fy, int tx, int ty, unsigned char c)
{
    boxf(_vram.bg, fx, fy, tx, ty, c);
}

/*
 *----------------------------------------------------------------------------
 * [VGE-API] vge_boxfSP: �X�v���C�g�ʂɓh��Ԃ��{�b�N�X��`�悷��
 *----------------------------------------------------------------------------
 * ����:
 * - fx [I] X���W(��_)
 * - fy [I] Y���W(��_)
 * - tx [I] X���W(�I�_)
 * - ty [I] Y���W(�I�_)
 * - c [I] �p���b�g�ԍ�
 *----------------------------------------------------------------------------
 */
void vge_boxfSP(int fx, int fy, int tx, int ty, unsigned char c)
{
    boxf(_vram.sp, fx, fy, tx, ty, c);
}

/*
 *----------------------------------------------------------------------------
 * vge_lineBG��vge_lineSP�̋��ʏ���
 *----------------------------------------------------------------------------
 */
static void boxf(unsigned char* p, int fx, int fy, int tx, int ty, unsigned char c)
{
    int w;
    /* form -> to�ϊ� */
    if (tx < fx) {
        w = fx;
        fx = tx;
        tx = w;
    }
    if (ty < fy) {
        w = fy;
        fy = ty;
        ty = w;
    }
    /* �͈͊O�`��̗}�~ */
    if (XSIZE <= fx || YSIZE <= fy || tx < 0 || ty < 0) {
        return;
    }
    /* �N���b�s���O */
    if (fx < 0) {
        fx = 0;
    }
    if (fy < 0) {
        fy = 0;
    }
    if (XSIZE <= tx) {
        tx = XSIZE - 1;
    }
    if (YSIZE <= ty) {
        ty = YSIZE - 1;
    }
    /* X�̕`��T�C�Y��\�ߋ��߂Ă��� */
    w = tx - fx;
    w++;
    /* �`�� */
    for (; fy <= ty; fy++) {
        memset(&p[fy * XSIZE + fx], c, w);
    }
}

/*
 *----------------------------------------------------------------------------
 * [VGE-API] �^�b�`�p�l���̏�Ԃ��擾
 *----------------------------------------------------------------------------
 * ����:
 * - s [O] �^�b�`�p�l������������Ԃ��ۂ��i��^�b�`0, �^�b�`��: �t���[�����j
 * - cx [O] ���݂�X���W�i��^�b�`���͕s��j
 * - cy [O] ���݂�Y���W�i��^�b�`���͕s��j
 * - dx [O] X�����̈ړ������i��^�b�`���͕s��j
 * - dy [O] Y�����̈ړ������i��^�b�`���͕s��j
 *----------------------------------------------------------------------------
 */
void vge_touch(int* s, int* cx, int* cy, int* dx, int* dy)
{
    *s = _touch.s;
    *cx = _touch.cx;
    *cy = _touch.cy;
    *dx = _touch.dx;
    *dy = _touch.dy;
    _touch.px = _touch.cx;
    _touch.py = _touch.cy;
    _touch.dx = 0;
    _touch.dy = 0;
}

/*
 *----------------------------------------------------------------------------
 * [VGE-API] vge_eload: ���ʉ�(�Ǝ��`��)���X���b�g�Ƀ��[�h����
 *----------------------------------------------------------------------------
 * ����:
 * - n [I] �X���b�g�ԍ�
 * - name [I] �t�@�C����
 *----------------------------------------------------------------------------
 * �߂�l: ������0�A���s���͔�0��Ԃ�.
 *----------------------------------------------------------------------------
 */
int vge_eload(unsigned char n, const char* name)
{
    unsigned char* bin;
    int size;
    int rc = -1;

    /* �Â��X���b�g����j�� */
    if (_eff[n].dat) {
        free(_eff[n].dat);
        _eff[n].dat = NULL;
        _eff[n].size = 0;
    }

    /* ROM�f�[�^�擾 */
    if (NULL == (bin = (unsigned char*)getbin(name, &size))) {
        goto ENDPROC;
    }

    /* �w�b�_���`�F�b�N */
    if ('E' != bin[0] || 'F' != bin[1] || 'F' != bin[2] || '\0' != bin[3]) {
        goto ENDPROC;
    }

    /* �T�C�Y�����z�X�g�o�C�g�I�[�_�Őݒ� */
    _eff[n].size = 0;
    _eff[n].size |= bin[4];
    _eff[n].size <<= 8;
    _eff[n].size |= bin[5];
    _eff[n].size <<= 8;
    _eff[n].size |= bin[6];
    _eff[n].size <<= 8;
    _eff[n].size |= bin[7];
    bin += 8;

    /* PCM�̈���m�� */
    if (NULL == (_eff[n].dat = (unsigned char*)malloc(_eff[n].size))) {
        goto ENDPROC;
    }

    /* PCM����ǂݍ��� */
    memcpy(_eff[n].dat, bin, _eff[n].size);

    /* �I������ */
    rc = 0;
ENDPROC:
    if (rc) {
        if (_eff[n].dat) {
            free(_eff[n].dat);
            _eff[n].dat = NULL;
        }
        _eff[n].size = 0;
    }
    return rc;
}

/*
 *----------------------------------------------------------------------------
 * [VGE-API] vge_eff: ���ʉ���炷
 *----------------------------------------------------------------------------
 * ����:
 * - n [I] �X���b�g�ԍ�
 *----------------------------------------------------------------------------
 * �߂�l: �Ȃ�
 *----------------------------------------------------------------------------
 */
void vge_eff(unsigned char n)
{
    if (_eff[n].dat) {
        eff_flag(&_eff[n], 1 + _eff[n].flag);
    }
}

/*
 *----------------------------------------------------------------------------
 * [VGE-API] vge_effstop: ���ʉ����~�߂�
 *----------------------------------------------------------------------------
 * ����:
 * - n [I] �X���b�g�ԍ�
 *----------------------------------------------------------------------------
 * �߂�l: �Ȃ�
 *----------------------------------------------------------------------------
 */
void vge_effstop(unsigned char n)
{
    if (_eff[n].dat) {
        eff_flag(&_eff[n], 0);
    }
}

/*
 *----------------------------------------------------------------------------
 * [VGE-API] vge_effstopA: �S�Ă̌��ʉ����~�߂�
 *----------------------------------------------------------------------------
 * ����: �Ȃ�
 *----------------------------------------------------------------------------
 * �߂�l: �Ȃ�
 *----------------------------------------------------------------------------
 */
void vge_effstopA()
{
    int i;
    for (i = 0; i < MAXSLOT; i++) {
        if (_eff[i].dat) {
            eff_flag(&_eff[i], 0);
        }
    }
}

/*
 *----------------------------------------------------------------------------
 * [VGE-API] vge_setmute: �������� / �炷�̐ݒ�
 *----------------------------------------------------------------------------
 * ����:
 * - n [I] ��0=�~���[�g�A0=����
 *----------------------------------------------------------------------------
 * �߂�l: �Ȃ�
 *----------------------------------------------------------------------------
 */
void vge_setmute(unsigned char n)
{
    _mute = n;
}

/*
 *----------------------------------------------------------------------------
 * [VGE-API] vge_getmute: �������� / �炷�̐ݒ���擾
 *----------------------------------------------------------------------------
 * �߂�l: ��0=�~���[�g�A0=����
 *----------------------------------------------------------------------------
 */
unsigned char vge_getmute()
{
    return _mute;
}

/*
 *----------------------------------------------------------------------------
 * [VGE-API] vge_bload: BGM(�Ǝ��`��)���X���b�g�Ƀ��[�h����
 *----------------------------------------------------------------------------
 * ����:
 * - n [I] �X���b�g�ԍ�
 * - name [I] �t�@�C����
 *----------------------------------------------------------------------------
 * �߂�l: ������0�A���s���͔�0��Ԃ�.
 *----------------------------------------------------------------------------
 */
int vge_bload(unsigned char n, const char* name)
{
    int size;
    _note[n] = (char*)getbin(name, &size);
    if (NULL == _note[n]) {
        return -1;
    }
    _notelen[n] = (uLong)size;
    return 0;
}

/*
 *----------------------------------------------------------------------------
 * [VGE-API] vge_bplay: BGM�����t����
 *----------------------------------------------------------------------------
 * ����:
 * - n [I] �X���b�g�ԍ�
 *----------------------------------------------------------------------------
 */
void vge_bplay(unsigned char n)
{
    vgsdec_load_bgm_from_memory(_psg, _note[n], _notelen[n]);
    vgsdec_set_value(_psg, VGSDEC_REG_RESET, 1);
    vgsdec_set_value(_psg, VGSDEC_REG_SYNTHESIS_BUFFER, 1);
    vgsdec_set_value(_psg, VGSDEC_REG_TIME, 0);
    _bstop = 0;
}

/*
 *----------------------------------------------------------------------------
 * [VGE-API] vge_bchk: BGM�����[�h����Ă��邩�`�F�b�N����
 *----------------------------------------------------------------------------
 * ����:
 * - n [I] �X���b�g�ԍ�
 *----------------------------------------------------------------------------
 */
int vge_bchk(unsigned char n)
{
    if (_note[n]) return 1;
    return 0;
}

/*
 *----------------------------------------------------------------------------
 * [VGE-API] vge_bstop: BGM�̉��t�𒆒f����
 *----------------------------------------------------------------------------
 */
void vge_bstop()
{
    _bstop = 1;
}

/*
 *----------------------------------------------------------------------------
 * [VGE-API] vge_bresume: BGM�̉��t�𒆒f�����Ƃ��납��ĊJ����
 *----------------------------------------------------------------------------
 */
void vge_bresume()
{
    _bstop = 0;
}

/*
 *----------------------------------------------------------------------------
 * [VGE-API] vge_bfade: BGM���w����g���̊Ԋu�Ńt�F�[�h�A�E�g
 *----------------------------------------------------------------------------
 */
void vge_bfade(unsigned int hz)
{
    vgsdec_set_value(_psg, VGSDEC_REG_FADEOUT, 1);
}

/*
 *----------------------------------------------------------------------------
 * [VGE-API] vge_setPause: �|�[�Y��Ԃ̐ݒ�E����
 *----------------------------------------------------------------------------
 * ����:
 * - p [I] �|�[�Y���
 *----------------------------------------------------------------------------
 */
void vge_setPause(unsigned char p)
{
    _pause = p;
}

/*
 *----------------------------------------------------------------------------
 * [VGE-API] vge_getdata: DSLOT�̃f�[�^���擾
 *----------------------------------------------------------------------------
 * ����:
 * - n [I] �X���b�g�ԍ�
 * - size [O] �f�[�^�T�C�Y
 *----------------------------------------------------------------------------
 * �߂�l: ��NULL=�f�[�^�̐擪�|�C���^�ANULL=�w��X���b�g�Ƀf�[�^�͖���
 *----------------------------------------------------------------------------
 */
const char* vge_getdata(unsigned char n, unsigned int* size)
{
    const char* ret;
    int size2;
    int* sp = (int*)size;
    char name[32];
    sprintf(name, "DSLOT%03d.DAT", (int)n);
    if (NULL == sp) {
        sp = &size2;
    }
    ret = getbin(name, sp);
    if (NULL == ret) {
        *sp = 0;
    }
    return ret;
}

/*
 *----------------------------------------------------------------------------
 * [VGE-API] vge_getTilt: �X�����擾
 *----------------------------------------------------------------------------
 * �߂�l: �}�X�^�{�����[��
 *----------------------------------------------------------------------------
 */
int vge_getTilt()
{
    return _tilt;
}

/* End of vgeapi.c */
