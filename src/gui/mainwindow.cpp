#include "mainwindow.h"
#include "ui_mainwindow.h"

// =================================================================================================

std::string updatePath (QDir old, QDir cur, std::string fname)
{
  if ( fname.size()==0 )
    return fname;

  return cur.relativeFilePath(old.filePath(QString::fromStdString(fname))).toStdString();
}

// =================================================================================================

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  // initialize GUI
  ui->setupUi(this);

  // always start on first tab
  ui->tabWidget->setCurrentIndex(0);

  // set default data (also useful to reduce the number of checks in the code)
  // - output images -> default assigned in "on_tab0_out_path_pushButton_clicked"
  data["output"]["result"] = "";
  data["output"]["interp"] = "";
  // - statistics settings
  data["stat"       ] = ""         ;
  data["nset"       ] = 1          ;
  data["periodic"   ] = true       ;
  data["zeropad"    ] = false      ;
  data["mask_weight"] = true       ;
  data["pixel_path" ] = "Bresenham";
  data["roi"        ] = {51,51}    ;
  // - empty file sets
  data["set0"]["field" ] = "";
  data["set0"]["dtype" ] = "";
  data["set0"]["files" ] = {};
  data["set0"]["config"] = {};
  data["set1"]["field" ] = "";
  data["set1"]["dtype" ] = "";
  data["set1"]["files" ] = {};
  data["set1"]["config"] = {};

  // tab3: link scroll position of graphicsViews
  QScrollBar *I1h = ui->tab3_image_graphicsView->horizontalScrollBar();
  QScrollBar *I2h = ui->tab3_phase_graphicsView->horizontalScrollBar();
  QScrollBar *I1v = ui->tab3_image_graphicsView->verticalScrollBar  ();
  QScrollBar *I2v = ui->tab3_phase_graphicsView->verticalScrollBar  ();
  connect(I1h,SIGNAL(valueChanged(int)),I2h,SLOT(setValue(int)));
  connect(I2h,SIGNAL(valueChanged(int)),I1h,SLOT(setValue(int)));
  connect(I1v,SIGNAL(valueChanged(int)),I2v,SLOT(setValue(int)));
  connect(I2v,SIGNAL(valueChanged(int)),I1v,SLOT(setValue(int)));

  // combine buttons to list, for fast references
  statBtn .push_back(ui->stat_S2_radioButton   ); statKey.push_back("S2"    );
  statBtn .push_back(ui->stat_C2_radioButton   ); statKey.push_back("C2"    );
  statBtn .push_back(ui->stat_L_radioButton    ); statKey.push_back("L"     );
  statBtn .push_back(ui->stat_W2_radioButton   ); statKey.push_back("W2"    );
  statBtn .push_back(ui->stat_W2c_radioButton  ); statKey.push_back("W2c"   );
  typeBtn .push_back(ui->tab1_im0b_radioButton ); typeKey.push_back("binary");
  typeBtn .push_back(ui->tab1_im0i_radioButton ); typeKey.push_back("int"   );
  typeBtn .push_back(ui->tab1_im0f_radioButton ); typeKey.push_back("float" );
  typeBtn .push_back(ui->tab1_im1b_radioButton ); typeKey.push_back("binary");
  typeBtn .push_back(ui->tab1_im1i_radioButton ); typeKey.push_back("int"   );
  typeBtn .push_back(ui->tab1_im1f_radioButton ); typeKey.push_back("float" );
  nsetBtn .push_back(ui->tab1_im0_checkBox     ); nsetKey.push_back("set0"  );
  nsetBtn .push_back(ui->tab1_im1_checkBox     ); nsetKey.push_back("set1"  );
  fileBtn .push_back(ui->tab2_im0Add_pushButton); fileBtnAdd.push_back(ui->tab2_im0Add_pushButton);
  fileBtn .push_back(ui->tab2_im1Add_pushButton); fileBtnAdd.push_back(ui->tab2_im1Add_pushButton);
  fileBtn .push_back(ui->tab2_im0Rmv_pushButton); fileBtnRmv.push_back(ui->tab2_im0Rmv_pushButton);
  fileBtn .push_back(ui->tab2_im1Rmv_pushButton); fileBtnRmv.push_back(ui->tab2_im1Rmv_pushButton);
  fileBtn .push_back(ui->tab2_im0Up__pushButton); fileBtnUp .push_back(ui->tab2_im0Up__pushButton);
  fileBtn .push_back(ui->tab2_im1Up__pushButton); fileBtnUp .push_back(ui->tab2_im1Up__pushButton);
  fileBtn .push_back(ui->tab2_im0Dwn_pushButton); fileBtnDwn.push_back(ui->tab2_im0Dwn_pushButton);
  fileBtn .push_back(ui->tab2_im1Dwn_pushButton); fileBtnDwn.push_back(ui->tab2_im1Dwn_pushButton);
  fileBtn .push_back(ui->tab2_im0Srt_pushButton); fileBtnSrt.push_back(ui->tab2_im0Srt_pushButton);
  fileBtn .push_back(ui->tab2_im1Srt_pushButton); fileBtnSrt.push_back(ui->tab2_im1Srt_pushButton);
  fileBtn .push_back(ui->tab2_cp_pushButton    );
  fileLst .push_back(ui->tab2_im0_listWidget   );
  fileLst .push_back(ui->tab2_im1_listWidget   );
  propLbl .push_back(ui->tab2_im0Phase_label   );
  propLbl .push_back(ui->tab2_im1Phase_label   );
  typeLbl .push_back(ui->tab2_im0Dtype_label   );
  typeLbl .push_back(ui->tab2_im1Dtype_label   );
  btnGroup.push_back(ui->buttonGroup           );
  btnGroup.push_back(ui->buttonGroup_tab1_im0  );
  btnGroup.push_back(ui->buttonGroup_tab1_im1  );

  // refresh tabs when tab is changed
  connect(ui->tabWidget,&QTabWidget::currentChanged,[=](){tab0_show();});
  connect(ui->tabWidget,&QTabWidget::currentChanged,[=](){tab1_show();});
  connect(ui->tabWidget,&QTabWidget::currentChanged,[=](){tab2_show();});

  // refresh file related views when JSON is loaded or "out_path" is changed
  connect(ui->tab0_out_path_pushButton,&QPushButton::clicked,this,[=](){tab0_show();});
  connect(ui->tab0_out_path_pushButton,&QPushButton::clicked,this,[=](){tab2_show();});
  connect(ui->tab0_load_pushButton    ,&QPushButton::clicked,this,[=](){tab0_show();});
  connect(ui->tab0_load_pushButton    ,&QPushButton::clicked,this,[=](){tab1_show();});
  connect(ui->tab0_load_pushButton    ,&QPushButton::clicked,this,[=](){tab2_show();});

  // tab1: button pressed -> update "data"
  for ( auto &i : statBtn ) connect(i,&QPushButton::clicked,this,[=](){tab1_read();});
  for ( auto &i : typeBtn ) connect(i,&QPushButton::clicked,this,[=](){tab1_read();});
  for ( auto &i : nsetBtn ) connect(i,&QPushButton::clicked,this,[=](){tab1_read();});

  // tab2: connect buttons to file manipulations -> update "data"
  for ( size_t i=0; i<2; ++i ) connect(fileBtnAdd[i],&QPushButton::clicked,this,[=](){fileAdd(i);});
  for ( size_t i=0; i<2; ++i ) connect(fileBtnRmv[i],&QPushButton::clicked,this,[=](){fileRmv(i);});
  for ( size_t i=0; i<2; ++i ) connect(fileBtnUp [i],&QPushButton::clicked,this,[=](){fileUp (i);});
  for ( size_t i=0; i<2; ++i ) connect(fileBtnDwn[i],&QPushButton::clicked,this,[=](){fileDwn(i);});
  for ( size_t i=0; i<2; ++i ) connect(fileBtnSrt[i],&QPushButton::clicked,this,[=](){fileSrt(i);});

  // tab2: button pressed -> refresh view with new "data"
  for ( auto &i : statBtn ) connect(i,&QPushButton::clicked,this,[=](){tab1_show();});
  for ( auto &i : typeBtn ) connect(i,&QPushButton::clicked,this,[=](){tab1_show();});
  for ( auto &i : nsetBtn ) connect(i,&QPushButton::clicked,this,[=](){tab1_show();});
  for ( auto &i : fileBtn ) connect(i,&QPushButton::clicked,this,[=](){tab2_show();});
}

// =================================================================================================

MainWindow::~MainWindow()
{
  delete ui;
}

// =================================================================================================

void MainWindow::promptWarning(QString msg)
{
  QMessageBox::warning(this,tr("GooseEYE"),msg,QMessageBox::Ok,QMessageBox::Ok);
}

// =================================================================================================

bool MainWindow::promptQuestion(QString msg)
{
  QMessageBox::StandardButton reply;

  reply = QMessageBox::question(this,tr("GooseEYE"),msg,QMessageBox::Yes|QMessageBox::No);

  if (reply == QMessageBox::Yes)
    return true;

  return false;
}

// =================================================================================================

bool MainWindow::exists(QString fname)
{
  QFileInfo finfo(out_path.filePath(fname));

  if ( finfo.exists() && finfo.isFile() )
    return true;
  else
    return false;
}

// =================================================================================================

size_t MainWindow::exists(QString f1, QString f2, QString f3)
{
  size_t out = 0;

  if ( exists(f1) ) { ++out; }
  if ( exists(f2) ) { ++out; }
  if ( exists(f3) ) { ++out; }

  return out;
}

// =================================================================================================

void MainWindow::on_tab0_load_pushButton_clicked()
{
  // load file using dialog
  QString fname = QFileDialog::getOpenFileName(
    this,
     tr("Open previously stored state"),
     QStandardPaths::writableLocation(QStandardPaths::HomeLocation),
     tr("JSON Files (*.json *.JSON)")
  );
  if ( !QFileInfo(fname).exists() )
    return;

  // try to see if the JSON file is corrupted, is so show error pop-up
  try {
    json tmp;
    std::ifstream inp(fname.toStdString());
    inp >> tmp;
  }
  catch (...) {
    promptWarning(tr("Unable to read JSON file"));
  }

  // convert JSON file to data object
  data.clear();
  std::ifstream inp(fname.toStdString());
  inp >> data;

  // store paths based in the path of the JSON file
  QFileInfo finfo(fname);
  out_path = finfo.dir();
  out_json = finfo.fileName();
}

// =================================================================================================

void MainWindow::on_tab0_out_path_pushButton_clicked()
{
  // back-up current working directory
  QString dirPrev = out_path.absolutePath();

  // open file-dialog
  QString dirName = QFileDialog::getExistingDirectory(
    this,
    tr("Open Directory"),
    dirPrev,
    QFileDialog::ShowDirsOnly
  );

  // no or non-existing directory selected -> quit function
  if ( dirName.size()==0 )
    return;
  if ( !QDir(dirName).exists() )
    return;

  // store directory
  out_path = QDir(dirName);

  // working directory changed -> convert relative paths to new relative paths
  if ( dirPrev.size()>0 && dirPrev!=dirName ) {
    for ( auto &set : nsetKey ) {
      for ( size_t j=0; j<data[set]["files"].size(); ++j ) {
        std::string prev = data[set]["files"][j];
        std::string next = updatePath(QDir(dirPrev),out_path,prev);
        data[set]["files" ][j   ] = next;
        data[set]["config"][next] = data[set]["config"][prev];
        data[set]["config"].erase(prev);
      }
    }
  }

  // default file-names
  QString f1 = "GooseEYE.json";
  QString f2 = "GooseEYE_result.pdf";
  QString f3 = "GooseEYE_result_interpretation.pdf";

  // if one of default files exists, find a number to append the files
  // (the same number if used for all files)
  if ( exists(f1,f2,f3) ) {
    f1 = "GooseEYE-%1.json";
    f2 = "GooseEYE-%1_result.pdf";
    f3 = "GooseEYE-%1_result_interpretation.pdf";
    int i = 1;
    while (exists(f1.arg(QString::number(i)),f2.arg(QString::number(i)),f3.arg(QString::number(i))))
      ++i;
    f1 = f1.arg(QString::number(i));
    f2 = f2.arg(QString::number(i));
    f3 = f3.arg(QString::number(i));
  }

  // store filenames
  out_json                 = f1;
  data["output"]["result"] = f2.toStdString();
  data["output"]["interp"] = f2.toStdString();
}

// =================================================================================================

void MainWindow::on_tab0_out_path_lineEdit_textEdited(const QString &arg1)
{
  out_path = QDir(arg1);
}

// =================================================================================================

void MainWindow::on_tab0_out_json_lineEdit_textEdited(const QString &arg1)
{
  out_json = arg1;
}

// =================================================================================================

void MainWindow::on_tab0_res1path_lineEdit_textEdited(const QString &arg1)
{
  data["output"]["result"] = arg1.toStdString();
}

// =================================================================================================

void MainWindow::on_tab0_res2path_lineEdit_textEdited(const QString &arg1)
{
  data["output"]["interp"] = arg1.toStdString();
}

// =================================================================================================

void MainWindow::tab0_show()
{
  ui->tab0_out_path_lineEdit->setText(out_path.absolutePath());
  ui->tab0_out_json_lineEdit->setText(out_json);
  ui->tab0_res1path_lineEdit->setText(QString::fromStdString(data["output"]["result"]));
  ui->tab0_res2path_lineEdit->setText(QString::fromStdString(data["output"]["interp"]));
}

// =================================================================================================

void MainWindow::tab1_read()
{
  // number of sets
  // - initialize
  size_t nset = 0;
  // - count
  for ( auto &i : nsetBtn )
    if ( i->isChecked() )
      ++nset;
  // - store
  data["nset"] = nset;
  // statistic
  for ( size_t i=0; i<statBtn.size(); ++i )
    if ( statBtn[i]->isChecked() )
      data["stat"] = statKey[i];
  // data-type
  for ( size_t j=0; j<2; ++j )
    for ( size_t i=0; i<3; ++i )
      if ( typeBtn[j*3+i]->isChecked() )
        data[nsetKey[j]]["dtype"] = typeKey[i];
  // miscellaneous settings
  data["periodic"   ] = ui->tab1_periodic_checkBox ->isChecked();
  data["zeropad"    ] = ui->tab1_zeropad_checkBox  ->isChecked();
  data["mask_weight"] = ui->tab1_maskW_checkBox    ->isChecked();
  data["pixel_path" ] = ui->tab1_pixelpath_comboBox->currentText().toStdString();
}

// =================================================================================================

void MainWindow::tab1_show()
{
  // clear all buttons
  for ( auto &i : btnGroup ) { i->setExclusive(false);                       }
  for ( auto &i : statBtn  ) { i->setChecked  (false); i->setEnabled(true ); }
  for ( auto &i : typeBtn  ) { i->setChecked  (false); i->setEnabled(false); }
  for ( auto &i : nsetBtn  ) { i->setChecked  (false); i->setEnabled(false); }
  for ( auto &i : btnGroup ) { i->setExclusive(true );                       }
  // fix number of sets
  if      ( data["stat"]=="L"                         ) { data["nset"] = 1;             }
  else if ( data["stat"]=="W2" || data["stat"]=="W2c" ) { data["nset"] = 2;             }
  else                                                  { nsetBtn[1]->setEnabled(true); }
  // fix data types
  int b0,i0,f0,b1,i1,f1;
  if      ( data["stat"]=="S2"  ) { b0=1; i0=0; f0=1; b1=1; i1=0; f1=1; }
  else if ( data["stat"]=="C2"  ) { b0=0; i0=1; f0=0; b1=0; i1=1; f1=0; }
  else if ( data["stat"]=="L"   ) { b0=1; i0=1; f0=0; b1=0; i1=0; f1=0; }
  else if ( data["stat"]=="W2"  ) { b0=1; i0=0; f0=1; b1=1; i1=0; f1=1; }
  else if ( data["stat"]=="W2c" ) { b0=0; i0=1; f0=0; b1=1; i1=0; f1=1; }
  else                            { b0=0; i0=0; f0=0; b1=0; i1=0; f1=0; }
  std::vector<int> dt;
  dt.push_back(b0);
  dt.push_back(i0);
  dt.push_back(f0);
  dt.push_back(b1);
  dt.push_back(i1);
  dt.push_back(f1);
  // read number of sets
  size_t n = data["nset"];
  // enable data-types
  for ( size_t i=0; i<typeBtn.size(); ++i )
    if ( dt[i] )
      typeBtn[i]->setEnabled(true);
  // read statistic
  for ( size_t i=0; i<statBtn.size(); ++i )
    if ( data["stat"]==statKey[i] )
      statBtn[i]->setChecked(true);
  // read data-type
  for ( size_t j=0; j<n; ++j )
    for ( size_t i=0; i<3; ++i )
      if ( data[nsetKey[j]]["dtype"]==typeKey[j*3+i] )
        if ( typeBtn[j*3+i]->isEnabled() )
          typeBtn[j*3+i]->setChecked(true);
  // set selected images
  for ( size_t j=0; j<n; ++j )
    nsetBtn[j]->setChecked(true);

  // selectively enable settings buttons
  // - defaults
  ui->tab1_periodic_checkBox   ->setEnabled(true );
  ui->tab1_zeropad_checkBox    ->setEnabled(true );
  ui->tab1_maskW_checkBox      ->setEnabled(false);
  ui->tab1_pixelpath_label     ->setEnabled(false);
  ui->tab1_pixelpath_comboBox  ->setEnabled(false);
  // - statistic specific: mask weights
  if ( data["stat"]=="W2" || data["stat"]=="W2c" )
    ui->tab1_maskW_checkBox    ->setEnabled(true);
  // - statistic specific: pixel path
  if ( data["stat"]=="L" || data["stat"]=="W2c" ) {
    ui->tab1_zeropad_checkBox  ->setEnabled(false);
    ui->tab1_pixelpath_label   ->setEnabled(true );
    ui->tab1_pixelpath_comboBox->setEnabled(true );
  }

  // apply settings
  QString pth = QString::fromStdString(data["pixel_path"]);
  ui->tab1_pixelpath_comboBox  ->setCurrentIndex(ui->tab1_pixelpath_comboBox->findText(pth));
  ui->tab1_periodic_checkBox   ->setChecked(data["periodic"   ]);
  ui->tab1_zeropad_checkBox    ->setChecked(data["zeropad"    ]);
  ui->tab1_maskW_checkBox      ->setChecked(data["mask_weight"]);
}

// =================================================================================================

std::vector<std::string> MainWindow::readFiles(size_t set)
{
  // local list of files
  std::vector<std::string> files;

  // copy items currently present in the list
  if ( data[nsetKey[set]].count("files") ) {
    for ( size_t i=0; i<data[nsetKey[set]]["files"].size(); ++i )
      files.push_back(data[nsetKey[set]]["files"][i]);
  }

  return files;
}

// =================================================================================================

void MainWindow::setFiles(size_t set, std::vector<std::string> files)
{
  if ( data[nsetKey[set]].count("files") ) {
    data[nsetKey[set]].erase("files");
  }

  data[nsetKey[set]]["files"] = files;
}

// =================================================================================================

void MainWindow::fileAdd(size_t set)
{
  // set filters to be used
  QStringList filters;
  filters << "Image files (*.png *.xpm *.jpg *.jpeg *.tif *.tiff *.bmp)"
          << "Any files (*)";

  // set-up read dialog
  QFileDialog dialog(this);
  dialog.setFileMode   (QFileDialog::ExistingFiles);
  dialog.setOption     (QFileDialog::HideNameFilterDetails,false);
  dialog.setDirectory  (out_path);
  dialog.setNameFilters(filters);
  dialog.setViewMode   (QFileDialog::List);

  // read list of files using dialog
  QStringList fileNames;
  if (dialog.exec())
    fileNames = dialog.selectedFiles();
  else
    return;

  // read current file selection
  std::vector<std::string> files = readFiles(set);

  // add selected items to the end of the list, but only if they are new
  for ( QString &file : fileNames ) {
    // - get the relative name of the selected file
    std::string fname = out_path.relativeFilePath(file).toStdString();
    // - check to add and add
    if ( ! ( std::find(files.begin(),files.end(),fname)!=files.end() ) )
      files.push_back(fname);
  }

  // store in data-structure
  setFiles(set,files);
}

// =================================================================================================

void MainWindow::fileRmv(size_t set)
{
  // read current file selection
  std::vector<std::string> files = readFiles(set);

  // initialize list that keep track which items have to be removed
  std::vector<int> rm(files.size(),0);

  // mark all select items for removal
  foreach ( QListWidgetItem *item, fileLst[set]->selectedItems() )
    rm[fileLst[set]->row(item)] = 1;

  // remove select items
  size_t j = 0;
  for ( size_t i=0; i<rm.size(); ++i ) {
    if ( rm[i] ) {
      files.erase(files.begin()+i-j);
      ++j;
    }
  }

  // store in data-structure
  setFiles(set,files);
}

// =================================================================================================

void MainWindow::fileUp(size_t set)
{
  // get selected items
  QList<QListWidgetItem*> items = fileLst[set]->selectedItems();

  // list with row-number of select items
  // - allocate
  std::vector<int> row(items.size());
  // - fill
  for ( int i=0; i<items.size(); ++i )
    row[i] = fileLst[set]->row(items[i]);

  // any items at the top of the list -> do nothing (quit function)
  for ( int i=0; i<items.size(); ++i )
    if ( row[i]<=0 )
      return;

  // sort row numbers, to ensure correct switching of items
  std::sort(row.begin(),row.end());

  // read current file list
  std::vector<std::string> files = readFiles(set);
  std::string              fname;

  // move selected rows one previous (up in the widget)
  for ( auto &i : row ) {
    fname      = files[i];
    files[i  ] = files[i-1];
    files[i-1] = fname;
  }

  // store in data-structure
  setFiles(set,files);

  // restore file selection
  // - clear entire selection
  for ( int j=0; j<fileLst[set]->count(); ++j )
    fileLst[set]->item(j)->setSelected(false);
  // - apply previous selection, moved one up
  for ( int j=0; j<items.size(); ++j )
    fileLst[set]->item(row[j]-1)->setSelected(true);
}

// =================================================================================================

void MainWindow::fileDwn(size_t set)
{
  // get selected items
  QList<QListWidgetItem*> items = fileLst[set]->selectedItems();

  // list with row-number of select items
  // - allocate
  std::vector<int> row(items.size());
  // - fill
  for ( int i=0 ; i<items.size() ; i++ )
    row[i] = fileLst[set]->row(items[i]);

  // any items at the bottom of the list -> do nothing (quit function)
  for ( int i=0 ; i<items.size() ; i++ )
    if ( row[i]>=fileLst[set]->count()-1 )
      return;

  // sort row numbers in descending order, to ensure correct switching of items
  std::sort(row.begin(),row.end(), [](int a, int b) { return a>b; });

  // read current file list
  std::vector<std::string> files = readFiles(set);
  std::string              fname;

  // move selected rows one forward (down in the widget)
  for ( auto &i : row ) {
    fname      = files[i+1];
    files[i+1] = files[i  ];
    files[i  ] = fname;
  }

  // store in data-structure
  setFiles(set,files);

  // restore file selection
  // - clear entire selection
  for ( int j=0; j<fileLst[set]->count(); ++j )
    fileLst[set]->item(j)->setSelected(false);
  // - apply previous selection, moved one up
  for ( int j=0; j<items.size(); ++j )
    fileLst[set]->item(row[j]+1)->setSelected(true);
}

// =================================================================================================

void MainWindow::fileSrt(size_t set)
{

  // read current file list
  std::vector<std::string> files = readFiles(set);

  // sort by name
  std::sort(files.begin(),files.end());

  // store in data-structure
  setFiles(set,files);

  // clear file selection
  for ( int j=0; j<fileLst[set]->count(); ++j )
    fileLst[set]->item(j)->setSelected(false);
}

// =================================================================================================

void MainWindow::on_tab2_cp_pushButton_clicked()
{
  // update the number of sets
  data["nset"] = 2;
  // copy all files
  setFiles(1,readFiles(0));
}

// =================================================================================================

void MainWindow::tab2_show()
{
  // disable all buttons by default (and clear labels), enable selectively below
  for ( auto &i : fileLst ) i->setEnabled(false);
  for ( auto &i : fileBtn ) i->setEnabled(false);
  for ( auto &i : propLbl ) i->setText   (""   );
  for ( auto &i : typeLbl ) i->setText   (""   );

  // for all sets
  for ( size_t set=0; set<data["nset"]; ++set ) {
    // - set labels
    try {
      propLbl[set]->setText(QString::fromStdString(data[nsetKey[set]]["field"]));
      typeLbl[set]->setText(QString::fromStdString(data[nsetKey[set]]["dtype"]));
    }
    catch (...) { }
    // - enable file list
    fileLst[set]->setEnabled(true);
    // - store selected items
    //   - read selected files
    QList<QListWidgetItem*> items = fileLst[set]->selectedItems();
    //   - allocate list with selected rows
    std::vector<int> row(items.size());
    //   - file list with selected rows
    for ( int j=0; j<items.size(); ++j )
      row[j] = fileLst[set]->row(items[j]);
    // - empty file lists
    while ( fileLst[set]->count()>0 )
      fileLst[set]->takeItem(0);
    // - read files
    std::vector<std::string> files = readFiles(set);
    // - fill file list
    for ( auto &file : files )
      fileLst[set]->addItem(QString::fromStdString(file));
    // - restore file selection
    for ( int j=0; j<fileLst[set]->count(); ++j )
      fileLst[set]->item(j)->setSelected(false);
    for ( int j=0; j<items.size(); ++j )
      fileLst[set]->item(row[j])->setSelected(true);
    // - enable buttons
    if ( true                     ) fileBtnAdd[set]       ->setEnabled(true);
    if ( files.size()>0           ) fileBtnRmv[set]       ->setEnabled(true);
    if ( files.size()>0           ) fileBtnUp [set]       ->setEnabled(true);
    if ( files.size()>0           ) fileBtnDwn[set]       ->setEnabled(true);
    if ( files.size()>0           ) fileBtnSrt[set]       ->setEnabled(true);
    if ( files.size()>0 && set==0 ) ui->tab2_cp_pushButton->setEnabled(true);
  }
}

// =================================================================================================

