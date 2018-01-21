/* Copyright 2012, Suzuki Plan.
   *----------------------------------------------------------------------------
   * [�T�v] VGE�����̃f�[�^�錾�ivge.cpp/vgeapi.c�Ԃŋ��L������Q�j
   *----------------------------------------------------------------------------
   */

#define MAXSLOT 256

#ifdef __cplusplus
extern "C" {
#endif

/*
 *----------------------------------------------------------------------------
 * VRAM���:
 * VGE��VRAM(Video RAM)�́A32�r�b�g����8�r�b�g�J���[(256�F).
 * �����̃V�X�e���́A32�r�b�g��32�r�b�g�J���[�����ʂ����ʂ���
 * �Ǝ��X�v���C�g�����̐��\������D�恦���AVGE���VRAM��256�F����Ƃ���.
 * (��1�s�N�Z��1�o�C�g�ɂȂ邽��)
 * �Ȃ��ABG�̈�͏�Ԃ�ێ����A�X�v���C�g�̈�̓t���[�����ɓ��e���N���A.
 * �����āA�X�v���C�g�̃p���b�g0�͓��߂��Ӗ�����.
 *----------------------------------------------------------------------------
 */
struct _VRAM {
    unsigned int pal[256];     /* �p���b�g�̈� */
    unsigned char bg[0x20000]; /* BG�̈� */
    unsigned char sp[0x20000]; /* �X�v���C�g�̈� */
};

/*
 *----------------------------------------------------------------------------
 * �X���b�g���
 *----------------------------------------------------------------------------
 */

/* �O���t�B�b�N */
struct _SLOT {
    int xs;
    int ys;
    unsigned char* dat;
};

/* ���ʉ� */
struct _EFF {
    unsigned int size;   /* �T�C�Y */
    unsigned short flag; /* �Đ��t���O */
    unsigned int pos;    /* �Đ��ʒu */
    unsigned char* dat;  /* PCM�f�[�^ */
};

/*
 *----------------------------------------------------------------------------
 * ���͏��
 *----------------------------------------------------------------------------
 */
struct _TOUCH {
    int s;  /* 1:�^�b�`��, 0:�����[�X�� */
    int t;  /* �^�b�`���Ă���t���[���� */
    int dx; /* X�����ړ��h�b�g�� */
    int dy; /* Y�����ړ��h�b�g�� */
    int cx; /* ���݂�X���W */
    int cy; /* ���݂�Y���W */
    int px; /* ���O��X���W */
    int py; /* ���O��Y���W */
};

/*
 *----------------------------------------------------------------------------
 * �O���[�o���ϐ���extern�錾
 *----------------------------------------------------------------------------
 */
extern struct _VRAM _vram;
extern struct _SLOT _slot[MAXSLOT];
extern unsigned short ADPAL[256];
extern struct _EFF _eff[MAXSLOT];
extern struct _TOUCH _touch;
extern unsigned char _mute;
extern unsigned char _pause;
extern short* TONE1[85];
extern short* TONE2[85];
extern short* TONE3[85];
extern short* TONE4[85];
extern int isIphone5;
extern void* _psg;
extern int _bstop;

/*
 *----------------------------------------------------------------------------
 * �����֐�(VGE/VGEAPI��)
 *----------------------------------------------------------------------------
 */
void eff_flag(struct _EFF* e, unsigned int f);
void eff_pos(struct _EFF* e, unsigned int f);
char* getbin(const char* name, int* size);
void vgsbuf(char* buf, size_t size);
size_t my_fread(void* buf, size_t size, size_t n, FILE* fp);
size_t my_fwrite(void* buf, size_t size, size_t n, FILE* fp);
void vgsint_setdir(const char* dir);
FILE* vge_fopen(const char* fname, const char* mode);

#ifdef __cplusplus
};
#endif
