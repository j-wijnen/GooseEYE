#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QSet>
#include <QMainWindow>
#include <QFileDialog>
#include <QRadioButton>
#include <QSpinBox>
#include <QStandardPaths>
#include <QMessageBox>
#include <QListWidget>
#include <QImage>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QByteArray>
#include <QObject>
#include <QScrollBar>
#include <QComboBox>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdlib>
#include <string>
#include <vector>
#include <math.h>

#include "../core/image.h"
#include "cppcolormap/cppcolormap.h"
#include "json/json.hpp"

using json = nlohmann::json;

// =================================================================================================

// convert relative path "fname" from being relative to "old" to begin relative to "current"
std::string updatePath (QDir old, QDir current, std::string fname);

// =================================================================================================

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

private slots:
  // directly connect buttons
  void on_pushButtonT0_load_clicked();                        // load JSON      -> update "data"
  void on_pushButtonT0_path_clicked();                    // change path    -> update "data"
  void on_lineEditT0_path_textEdited(const QString &arg1);// change path    -> update "data"
  void on_lineEditT0_json_textEdited(const QString &arg1);// manual file-name change
  void on_tab0_res1path_lineEdit_textEdited(const QString &arg1);// manual file-name change
  void on_tab0_res2path_lineEdit_textEdited(const QString &arg1);// manual file-name change
  void on_tab2_cp_pushButton_clicked();                          // copy files "set0" -> "set1"
  // connect groups of buttons
  void fileAdd(size_t); // add    files         -> update "data"
  void fileRmv(size_t); // remove files         -> update "data"
  void fileUp (size_t); // move   files up      -> update "data"
  void fileDwn(size_t); // move   files down    -> update "data"
  void fileSrt(size_t); // sort   files by name -> update "data"
  // synchronize widgets and "data"
  void tab1_read(); // update "data" with new information from widgets
  void tab0_show(); // refresh with new "data"
  void tab1_show(); // refresh with new "data"
  void tab2_show(); // refresh with new "data"
  void tab3_show(); // refresh with new "data"
  void tab3_read(size_t idx);
  // support functions
  // - read single file "iimg" from "iset", output as Qt string
  QString readFile(size_t iset, size_t iimg);
  // - read files from "iset", output as vector of strings
  std::vector<std::string> readFiles(size_t iset);
  // - replace all files in "iset" with vector of strings
  void setFiles(size_t iset, std::vector<std::string> files);
  // - compute scale factor, to show the image widget spanning
  double tab3_scaleImage();
  // - read all image settings (phase,mask1,mask2,mask3,row,col) as a single vector (min,max,...)
  std::vector<size_t> imageSettings(size_t iset, size_t iimg);
  // - read image from file
  std::tuple<mat::matrix<int>,QImage> readImage(size_t iset, size_t iimg);
  // - interpret image
  std::tuple<mat::matrix<int>,mat::matrix<int>> interpretImage(
    size_t iset, size_t iimg, mat::matrix<int> im, int crop=-1 );
  // - read and interpret image
  std::tuple<mat::matrix<int>,mat::matrix<int>> interpretImage(
    size_t iset, size_t iimg, int crop=-1 );

private:
  Ui::MainWindow  *ui;
  json             data;
  QDir             out_path = QDir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
  QString          out_json = "";
  int              iset_    = -1;        // store last read set/image
  int              iimg_    = -1;        // store last read set/image
  mat::matrix<int> imgRaw_      ;        // store image to view interpreted image without reread
  std::vector<QRadioButton*> statBtn;    // list with buttons, each corresponding to one statistic
  std::vector<std::string>   statKey;    // names corresponding to "statBtn"
  std::vector<QRadioButton*> typeBtn;    // list with buttons with dtypes: [b0,i0,f0,b1,i1,f1]
  std::vector<std::string>   typeKey;    // names corresponding to "typeBtn"
  std::vector<QCheckBox*>    nsetBtn;    // list with buttons to select the number of sets
  std::vector<std::string>   nsetKey;    // names corresponding to "nsetBtn": ["set0","set1"]
  std::vector<QPushButton*>  fileBtn;    // list with all file manipulation buttons (tab2)
  std::vector<QPushButton*>  fileBtnAdd; // - add buttons ["set0","set1"]
  std::vector<QPushButton*>  fileBtnRmv; // - add remove buttons
  std::vector<QPushButton*>  fileBtnUp ; // - all move up buttons
  std::vector<QPushButton*>  fileBtnDwn; // - all move down buttons
  std::vector<QPushButton*>  fileBtnSrt; // - all sort buttons
  std::vector<QListWidget*>  fileLst;    // all file lists
  std::vector<QLabel*>       propLbl;    // list with labels to denote the field-type ["phase",...]
  std::vector<QLabel*>       typeLbl;    // list with labels to denote the data-type
  std::vector<std::string>   imgCheckLbl;
  std::vector<QSpinBox*>     imgSpin;
  std::vector<QCheckBox*>    imgCheck;
  std::vector<QPushButton*>  imgBtn;     // list with all image selection buttons
  std::vector<QButtonGroup*> btnGroup;   // list with all groups of radioButtons
  bool   promptQuestion(QString);        // prompt question to user (return response)
  void   promptWarning (QString);        // prompt warning to user
  bool   exists(QString);                // check if a file exits (NB: relative path wrt "out_path")
  size_t exists(QString,QString,QString);// check how many of these files exist
};

#endif // MAINWINDOW_H
