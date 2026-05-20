#if defined(__APPLE__)
#  define GL_SILENCE_DEPRECATION
#  include <GLUT/glut.h>
#  include <OpenGL/glext.h>
#else
#  if defined(_MSC_VER)
//#    pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
#    define _USE_MATH_DEFINES
#    define _CRT_SECURE_NO_WARNINGS
#  endif
#  include <GL/glut.h>
#  include <GL/glext.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* トラックボール処理用関数の宣言 */
#include "trackball.h"

/* 箱を描く関数の宣言 */
#include "box.h"

/*
** 光源
*/
static const GLfloat lightpos[] = { 0.0f, 0.0f, 1.0f, 0.0f }; /* 位置　　　 */
static const GLfloat lightcol[] = { 1.0f, 1.0f, 1.0f, 1.0f }; /* 直接光強度 */
static const GLfloat lightamb[] = { 0.1f, 0.1f, 0.1f, 1.0f }; /* 環境光強度 */

/*
** テクスチャ
*/
#define TEXWIDTH  128                               /* テクスチャの幅　　　 */
#define TEXHEIGHT 128                               /* テクスチャの高さ　　 */
static const char *texture_files[] = {              /* テクスチャファイル名 */
  "room2nx.raw",
  "room2ny.raw",
  "room2nz.raw",
  "room2px.raw",
  "room2py.raw",
  "room2pz.raw",
};
static const int target[] = {                   /* テクスチャのターゲット名 */
  GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
  GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
  GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
  GL_TEXTURE_CUBE_MAP_POSITIVE_X,
  GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
  GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
};

/*
** 初期化
*/
static void init()
{
  /* テクスチャ画像はワード単位に詰め込まれている */
  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

  /* テクスチャの読み込みに使う配列 */
  GLubyte texture[TEXHEIGHT][TEXWIDTH][4];

  /* テクスチャ画像の読み込み */
  for (int i = 0; i < 6; ++i) {
    FILE* fp = fopen(texture_files[i], "rb");
    if (fp != NULL) {
      fread(texture, sizeof texture, 1, fp);
      fclose(fp);
    }
    else {
      perror(texture_files[i]);
    }

    /* テクスチャの割り当て */
    glTexImage2D(target[i], 0, GL_RGBA, TEXWIDTH, TEXHEIGHT, 0,
      GL_RGBA, GL_UNSIGNED_BYTE, texture);
  }

  /* テクスチャを拡大・縮小する方法の指定 */
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  /* テクスチャの繰り返し方法の指定 */
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  /* テクスチャ環境 */
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

#if 0
  /* 混合する色の設定 */
  static const GLfloat blend[] = { 0.0f, 1.0f, 0.0f, 1.0f };
  glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, blend);
#endif

  /* キューブマッピング用のテクスチャ座標を生成する */
  glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
  glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
  glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);

  /* 初期設定 */
  glClearColor(0.3f, 0.3f, 1.0f, 0.0f);
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);

  /* 光源の初期設定 */
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, lightcol);
  glLightfv(GL_LIGHT0, GL_SPECULAR, lightcol);
  glLightfv(GL_LIGHT0, GL_AMBIENT, lightamb);
}

/*
** シーンの描画
*/
static void scene()
{
  static const GLfloat color[] = { 1.0f, 1.0f, 1.0f, 1.0f };   /* 材質 (色) */

  /* 材質の設定 */
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);

  /* テクスチャマッピング開始 */
  glEnable(GL_TEXTURE_CUBE_MAP);

  /* テクスチャ座標の自動生成を有効にする */
  glEnable(GL_TEXTURE_GEN_S);
  glEnable(GL_TEXTURE_GEN_T);
  glEnable(GL_TEXTURE_GEN_R);

  /* ティーポットを描く */
  glutSolidTeapot(1.0);

  /* テクスチャ座標の自動生成を無効にする */
  glDisable(GL_TEXTURE_GEN_S);
  glDisable(GL_TEXTURE_GEN_T);
  glDisable(GL_TEXTURE_GEN_R);

  /* テクスチャマッピング終了 */
  glDisable(GL_TEXTURE_CUBE_MAP);
}

/****************************
** GLUT のコールバック関数 **
****************************/

static void display()
{
  /* モデルビュー変換行列の設定 */
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  /* 光源の位置を設定 */
  glLightfv(GL_LIGHT0, GL_POSITION, lightpos);

  /* 視点の移動（物体の方を奥に移動）*/
  glTranslated(0.0, 0.0, -3.0);
  //gluLookAt(1.5, 2.0, 2.5, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

  /* トラックボール処理で図形を回転 */
  //glMultMatrixd(trackballRotation());

  /* テクスチャ行列の設定 */
  glMatrixMode(GL_TEXTURE);
  glLoadIdentity();

  /* トラックボール処理でテクスチャを回転 */
  glMultMatrixd(trackballRotation());

  /* 画面クリア */
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  /* シーンの描画 */
  scene();

  /* ダブルバッファリング */
  glutSwapBuffers();
}

static void resize(int w, int h)
{
  /* トラックボールする範囲 */
  trackballRegion(w, h);

  /* ウィンドウ全体をビューポートにする */
  glViewport(0, 0, w, h);

  /* 透視変換行列の指定 */
  glMatrixMode(GL_PROJECTION);

  /* 透視変換行列の初期化 */
  glLoadIdentity();
  gluPerspective(60.0, (double)w / (double)h, 0.1, 10.0);
}

static void idle()
{
  /* 画面の描き替え */
  glutPostRedisplay();
}

static void mouse(int button, int state, int x, int y)
{
  switch (button) {
  case GLUT_LEFT_BUTTON:
    switch (state) {
    case GLUT_DOWN:
      /* トラックボール開始 */
      trackballStart(x, y);
      glutIdleFunc(idle);
      break;
    case GLUT_UP:
      /* トラックボール停止 */
      trackballStop(x, y);
      glutIdleFunc(0);
      break;
    default:
      break;
    }
    break;
  default:
    break;
  }
}

static void motion(int x, int y)
{
  /* トラックボール移動 */
  trackballMotion(x, y);
}

static void keyboard(unsigned char key, int x, int y)
{
  switch (key) {
  case 'q':
  case 'Q':
  case '\033':
    /* ESC か q か Q をタイプしたら終了 */
    exit(0);
  default:
    break;
  }
}

/*
** メインプログラム
*/
int main(int argc, char* argv[])
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
  glutCreateWindow(argv[0]);
  glutDisplayFunc(display);
  glutReshapeFunc(resize);
  glutMouseFunc(mouse);
  glutMotionFunc(motion);
  glutKeyboardFunc(keyboard);
  init();
  glutMainLoop();
  return 0;
}
