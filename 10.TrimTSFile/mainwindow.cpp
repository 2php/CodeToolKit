#include "mainwindow.h"
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{    
    main_layout = new QVBoxLayout(this);
    tool_layout = new QHBoxLayout(this);
    repeat_title_layout = new QVBoxLayout(this);
    output_title_layout = new QVBoxLayout(this);
    content_layout = new QHBoxLayout(this);
    save_layout = new QHBoxLayout(this);

    url_label = new QLabel(this);//语言文件
    url_linedit = new QLineEdit(this);//语言文件
    url_button = new QPushButton(this);//选择文件
    recall_button = new QPushButton(this);//回收翻译
    doall_chkbox = new QCheckBox(this);//全部替换
    trim_button = new QPushButton(this);//清理操作
    cancel_button = new QPushButton(this);

    repeat_title_label = new QLabel(this);
    repeat_treewidget = new QTreeWidget(this);//重复树
    output_title_label = new QLabel(this);
    output_treewidget = new QTreeWidget(this);//输出树

    tool_layout->setContentsMargins(0,0,0,0);
    tool_layout->addWidget(url_label);
    tool_layout->addWidget(url_linedit);
    tool_layout->addWidget(url_button);

    repeat_title_layout->setContentsMargins(0,0,0,0);
    repeat_title_layout->addWidget(repeat_title_label);
    repeat_title_layout->addWidget(repeat_treewidget);

    output_title_layout->setContentsMargins(0,0,0,0);
    output_title_layout->addWidget(output_title_label);
    output_title_layout->addWidget(output_treewidget);

    content_layout->setContentsMargins(0,0,0,0);
    content_layout->addLayout(repeat_title_layout);
    content_layout->addWidget(recall_button);
    content_layout->addLayout(output_title_layout);

    save_layout->setContentsMargins(0,0,0,0);
    save_layout->addWidget(doall_chkbox);
    save_layout->addStretch(0);
    save_layout->addWidget(trim_button);
    save_layout->addWidget(cancel_button);

    main_layout->setContentsMargins(20,20,20,20);
    main_layout->addLayout(tool_layout);
    main_layout->addSpacing(10);
    main_layout->addLayout(content_layout);
    main_layout->addSpacing(10);
    main_layout->addLayout(save_layout);

    this->setLayout(main_layout);
    this->initControl();
    this->bindEvent();    
}

void MainWindow::initControl()
{
    url_label->setText(tr("language file:"));//语言文件
    url_button->setText(tr("choose..."));//选择文件
    recall_button->setText(">>");
    recall_button->setToolTip(tr("recall translation"));//回收翻译
    doall_chkbox->setText(tr("recall all"));//全部替换
    trim_button->setText(tr("save"));//清理重复
    cancel_button->setText(tr("cancel"));

    repeat_title_label->setText(tr("repeat items"));//重复的数据
    repeat_title_label->setAlignment(Qt::AlignCenter);
    output_title_label->setText(tr("output items"));//输出的数据
    output_title_label->setAlignment(Qt::AlignCenter);

    QStringList headLabels;
    headLabels<<tr("source language")<<tr("target language");

    repeat_treewidget->setColumnCount(2);
    repeat_treewidget->setHeaderLabels(headLabels);
    //repeat_treewidget->header()->setSectionResizeMode(QHeaderView::Interactive);
    repeat_treewidget->headerItem()->setTextAlignment(0, Qt::AlignCenter);
    repeat_treewidget->headerItem()->setTextAlignment(1, Qt::AlignCenter);
    repeat_treewidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    repeat_treewidget->setSelectionMode(QAbstractItemView::SingleSelection);
    repeat_treewidget->setFocusPolicy(Qt::NoFocus);

    output_treewidget->setColumnCount(2);
    output_treewidget->setHeaderLabels(headLabels);
    //output_treewidget->header()->setSectionResizeMode(QHeaderView::Interactive);
    output_treewidget->headerItem()->setTextAlignment(0, Qt::AlignCenter);
    output_treewidget->headerItem()->setTextAlignment(0, Qt::AlignCenter);
    output_treewidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    output_treewidget->setSelectionMode(QAbstractItemView::SingleSelection);
    output_treewidget->setFocusPolicy(Qt::NoFocus);

    QString style =
            "QTreeView{ background:#ffffff;color:#202020; }"
            "QTreeView::item{ height:30px; }"
            "QTreeView::item:disabled{ color:#cccccc; }"
            "QTreeView::item:hover{ background-color:rgb(0,255,0,30); }"
            "QTreeView::item:selected{ background-color:rgb(30,0,80,80); }";
    repeat_treewidget->setStyleSheet(style);
    output_treewidget->setStyleSheet(style);
}

void MainWindow::bindEvent()
{
    connect( url_button,SIGNAL(clicked()),this,SLOT(onSlotChooseLangFileFunc()) );

    connect( recall_button,SIGNAL(clicked()),this,SLOT(onSlotRecallRepeatTranslationFunc()) );
    connect( trim_button,SIGNAL(clicked()),this,SLOT(onSlotTrimLangFileContextFunc()) );

    connect( cancel_button,SIGNAL(clicked()),this,SLOT(close()) );

    connect( repeat_treewidget,SIGNAL(clicked(QModelIndex)),this,SLOT(onSlotSelectedRightItemFunc(QModelIndex)) );
}

void MainWindow::onSlotChooseLangFileFunc()
{
    QString langFile = QFileDialog::getOpenFileName(0, tr("choose language file"), tr("."), "qt ts language(*.ts)");
    if( langFile.length()<=0 )
    {
        return ;
    }

    url_linedit->setText(langFile);

    this->prepareRepeatXml();
    this->prepareOutputXml();
}

//召回重复的翻译选项
void MainWindow::onSlotRecallRepeatTranslationFunc()
{
    QTreeWidgetItem* item = repeat_treewidget->currentItem();
    if( !item )//没有选中
    {
        if( doall_chkbox->isChecked() )
        {
            if( QMessageBox::Yes==QMessageBox::question(0, tr("notice"), tr("will replace all the existing translation! continue?")) )
            {
                QTreeWidgetItemIterator it(repeat_treewidget);
                while( *it )//召回全部项
                {
                    this->recallItemTranslation(*it);
                    ++it;
                }
            }
        }
        else
        {
            QMessageBox::warning(0, tr("notice"), tr("select an repeat translation!"));
        }
        return ;
    }

    //召回选中项
    this->recallItemTranslation(item);
}

//保存召回的翻译XML
void MainWindow::onSlotTrimLangFileContextFunc()
{
    QString langUrl = url_linedit->text();
    if( langUrl.length()<=0 )
    {
        QMessageBox::warning(0, tr("notice"), tr("language url is null!"));
        return ;
    }

    //保存修改后的
    langUrl += ".mod.ts";
    QFile ofile(langUrl);
    if( !ofile.open(QIODevice::WriteOnly | QIODevice::Truncate |QIODevice::Text) )
    {
        return ;
    }
    QTextStream out(&ofile);
    out.setCodec("UTF-8");
    output_doc.save(out, 4, QDomNode::EncodingFromTextStream);
    ofile.close();
}

void MainWindow::onSlotSelectedRightItemFunc(QModelIndex)
{
    QTreeWidgetItem* item = repeat_treewidget->currentItem();
    if( !item )
    {
        return ;
    }

    output_treewidget->setCurrentItem(NULL);//取消选择

    //选择节点的父节点
    QTreeWidgetItem* itemparent = item->parent();
    if( !itemparent )
    {
        return ;
    }
    //qDebug()<<"当前节点——"<<item->text(0)<<"其父节点——"<<itemparent->text(0);

    //遍历右边树
    QTreeWidgetItemIterator it(output_treewidget);
    while( *it )
    {
        if( (*it)->text(0).compare(item->text(0), Qt::CaseSensitive)==0 && !(*it)->text(1).isEmpty() )//节点关键字
        {
            QTreeWidgetItem* itemcmp = (*it)->parent();//父节点关键字
            if( itemcmp && itemcmp->text(0).compare(itemparent->text(0), Qt::CaseSensitive)==0 )
            {
                //qDebug()<<"右边当前节点——"<<(*it)->text(0)<<"其父节点——"<<itemcmp->text(0);
                output_treewidget->setCurrentItem(*it);//选中对应项
                break;
            }
        }
        ++it;
    }

}

void MainWindow::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);

    repeat_treewidget->setColumnWidth(0, repeat_treewidget->width()*0.5);
    output_treewidget->setColumnWidth(0, output_treewidget->width()*0.5);
}

void MainWindow::recallItemTranslation(QTreeWidgetItem *item)
{
    if( !item )//重复选项
    {
        return ;
    }

    //选中对应的右边选项，替换其翻译
    repeat_treewidget->setCurrentItem(item);
    repeat_treewidget->clicked(repeat_treewidget->currentIndex());
    if( output_treewidget->currentItem() )
    {
        output_treewidget->currentItem()->setText(1, item->text(1));
        output_treewidget->currentItem()->setToolTip(0, item->text(1));
    }

    //获取父节点名称
    QTreeWidgetItem* itemparent = item->parent();
    if( !itemparent )
    {
        qDebug()<<"节点父节点未找到——"<<item->text(0);
        return ;
    }

    //修改去重后的翻译XML
    if( output_doc.isNull() )
    {
        qDebug()<<"XML----"<<output_doc.isNull();
        return ;
    }

    QDomElement root = output_doc.documentElement();//根节点
    QDomNode node = root.firstChild();
    while( !node.isNull() )
    {
        QDomElement ele = node.toElement();
        if( !ele.isNull() && ele.tagName().compare("context", Qt::CaseInsensitive)==0 )
        {
            //qDebug()<<"遍历节点——"<<ele.tagName();
            QDomElement e = ele.firstChildElement();//子节点
            while( !e.isNull() )
            {
                if( e.tagName().compare("name",Qt::CaseInsensitive)==0 )
                {
                    if( e.text().compare(itemparent->text(0), Qt::CaseSensitive)!=0 )
                    {
                        break;
                    }
                    //qDebug()<<"XML中找到节点——"<<e.text()<<itemparent->text(0);
                }
                if( e.tagName().compare("message",Qt::CaseInsensitive)==0 )
                {
                    QDomElement eTran = e.firstChildElement();
                    while( !eTran.isNull() )
                    {
                        //qDebug()<<"1111XML中找到翻译——"<<eTran.tagName()<<item->text(0);
                        if( eTran.tagName().compare("source",Qt::CaseSensitive)==0 )
                        {
                            if( eTran.text().compare(item->text(0),Qt::CaseSensitive)==0 )
                            {
                                //qDebug()<<"2222XML中找到翻译——"<<eTran.text()<<item->text(0);
                                eTran = eTran.nextSiblingElement("translation");
                                if( !eTran.isNull() )
                                {
                                    //eTran.setNodeValue(item->text(1));//设置节点数据，没作用
                                    eTran.firstChild().setNodeValue(item->text(1));//修改节点数据，可以
                                    qDebug()<<"3333XML中写入翻译——"<<eTran.tagName()<<eTran.text();
                                }
                                break;
                            }
                        }
                        eTran = eTran.nextSiblingElement();
                    }
                }
                e = e.nextSiblingElement();
            }//
        }
        node = node.nextSibling();
    }

}

void MainWindow::readAndSave()
{
    QString langUrl = url_linedit->text();
    if( langUrl.length()<=0 )
    {
        QMessageBox::warning(0, tr("notice"), tr("language url is null!"));
        return ;
    }

    QFile file(langUrl);
    if( !file.open(QFile::ReadOnly|QFile::Text) )
    {
        QMessageBox::warning(0, tr("notice"), tr("language file open failed!"));
        return ;
    }

    QDomDocument doc;
    if( !doc.setContent(&file) )
    {
        file.close();
        return ;
    }
    file.close();

    if( doc.isNull() )
    {
        qDebug()<<"XML----"<<doc.isNull();
        return ;
    }

    QDomElement root = doc.documentElement();
    QString rootName = root.tagName();
    if( rootName.compare("ts", Qt::CaseInsensitive)==0 )
    {
        //qDebug()<<"version---"<<rootName<<root.hasAttribute("version");
    }

    QDomNode node = root.firstChild();
    while( !node.isNull() )
    {
        QDomElement ele = node.toElement();
        if( !ele.isNull() && ele.tagName().compare("context", Qt::CaseInsensitive)==0 )
        {
            //qDebug()<<"遍历节点——"<<ele.tagName();
            QDomElement e = ele.firstChildElement();//子节点
            while( !e.isNull() )
            {
                if( e.tagName().compare("name",Qt::CaseInsensitive)==0 )
                {
                    //qDebug()<<"to translate file----"<<e.tagName()<<e.text();
                }
                if( e.tagName().compare("message",Qt::CaseInsensitive)==0 )
                {
                    QDomElement eLoc = e.firstChildElement("location");
                    if( eLoc.isNull() )
                    {
                        //qDebug()<<"找不到location节点，就是应该删除的message项！";
                        QDomElement temp = e;
                        e = e.nextSiblingElement();
                        ele.removeChild(temp);
                        continue;
                    }
                    else
                    {
                        //qDebug()<<"find lang file location----"<<eLoc.attribute("filename")
                        //       <<eLoc.attribute("line");
                    }
                }
                e = e.nextSiblingElement();
            }//
        }
        node = node.nextSibling();
    }

    //保存修改后的
    langUrl += ".mod.ts";
    QFile ofile(langUrl);
    if( !ofile.open(QIODevice::WriteOnly | QIODevice::Truncate |QIODevice::Text) )
    {
        return ;
    }
    QTextStream out(&ofile);
    out.setCodec("UTF-8");
    doc.save(out, 4, QDomNode::EncodingFromTextStream);
    ofile.close();
}

void MainWindow::prepareRepeatXml()
{
    QString langUrl = url_linedit->text();
    if( langUrl.length()<=0 )
    {
        QMessageBox::warning(0, tr("notice"), tr("language url is null!"));
        return ;
    }

    repeat_treewidget->clear();

    QFile file(langUrl);
    if( !file.open(QFile::ReadOnly|QFile::Text) )
    {
        QMessageBox::warning(0, tr("notice"), tr("language file open failed!"));
        return ;
    }

    QDomDocument doc;
    if( !doc.setContent(&file) )
    {
        file.close();
        return ;
    }
    file.close();

    if( doc.isNull() )
    {
        qDebug()<<"XML----"<<doc.isNull();
        return ;
    }

    QTreeWidgetItem* itemRoot = NULL;//树根节点

    QDomElement root = doc.documentElement();
    if( root.tagName().compare("ts", Qt::CaseInsensitive)==0 )
    {
        itemRoot = new QTreeWidgetItem(0);//根节点
        itemRoot->setText(0, root.tagName());
        repeat_treewidget->addTopLevelItem(itemRoot);
    }

    QDomNode node = root.firstChild();
    while( !node.isNull() )
    {
        QDomElement ele = node.toElement();
        if( !ele.isNull() && ele.tagName().compare("context", Qt::CaseInsensitive)==0 )
        {
            //qDebug()<<"遍历节点——"<<ele.tagName();
            QTreeWidgetItem* itemFile = NULL;//文件节点
            QString nodeName;//文件节点名称

            QDomElement e = ele.firstChildElement();//子节点
            while( !e.isNull() )
            {
                if( e.tagName().compare("name",Qt::CaseInsensitive)==0 )
                {
                    nodeName = e.text();
                }
                if( e.tagName().compare("message",Qt::CaseInsensitive)==0 )
                {
                    QDomElement eLoc = e.firstChildElement("location");//文中位置
                    if( eLoc.isNull() )
                    {
                        itemFile = new QTreeWidgetItem(0);//context name节点
                        itemFile->setText(0, nodeName);
                        itemRoot->addChild(itemFile);

                        QStringList stringLang;//语言翻译数据
                        eLoc = e.firstChildElement();//再次取值
                        while( !eLoc.isNull() )
                        {
                            //qDebug()<<eLoc.tagName()<<eLoc.text();
                            if( eLoc.tagName().compare("source",Qt::CaseInsensitive)==0 ||
                                eLoc.tagName().compare("translation",Qt::CaseInsensitive)==0 )
                            {
                                stringLang<<eLoc.text();
                            }
                            eLoc = eLoc.nextSiblingElement();
                        }

                        if( itemFile )//添加语言翻译
                        {
                            QTreeWidgetItem* itemLang = new QTreeWidgetItem(0);//context message location节点
                            itemLang->setText(0, stringLang.first());
                            itemLang->setToolTip(0, stringLang.last());
                            itemLang->setText(1, stringLang.last());
                            itemFile->addChild(itemLang);
                        }

                        //移除失效的节点
                        //QDomElement temp = e; e = e.nextSiblingElement();
                        //ele.removeChild(temp);
                        //continue;
                    }
                }
                e = e.nextSiblingElement();
            }//
        }
        node = node.nextSibling();
    }

    //展开所有项
    repeat_treewidget->expandAll();
}

void MainWindow::prepareOutputXml()
{
    QString langUrl = url_linedit->text();
    if( langUrl.length()<=0 )
    {
        QMessageBox::warning(0, tr("notice"), tr("language url is null!"));
        return ;
    }

    output_treewidget->clear();

    QFile file(langUrl);
    if( !file.open(QFile::ReadOnly|QFile::Text) )
    {
        QMessageBox::warning(0, tr("notice"), tr("language file open failed!"));
        return ;
    }

    QDomDocument doc;
    if( !doc.setContent(&file) )
    {
        file.close();
        return ;
    }
    file.close();

    if( doc.isNull() )
    {
        qDebug()<<"XML----"<<doc.isNull();
        return ;
    }

    QTreeWidgetItem* itemRoot = NULL;//树根节点

    QDomElement root = doc.documentElement();
    if( root.tagName().compare("ts", Qt::CaseInsensitive)==0 )
    {
        itemRoot = new QTreeWidgetItem(0);//根节点
        itemRoot->setText(0, root.tagName());
        output_treewidget->addTopLevelItem(itemRoot);
    }

    QDomNode node = root.firstChild();
    while( !node.isNull() )
    {
        QDomElement ele = node.toElement();
        if( !ele.isNull() && ele.tagName().compare("context", Qt::CaseInsensitive)==0 )
        {
            //qDebug()<<"遍历节点——"<<ele.tagName();
            QTreeWidgetItem* itemFile = NULL;//文件节点
            QString nodeName;//文件节点名称

            QDomElement e = ele.firstChildElement();//子节点
            while( !e.isNull() )
            {
                if( e.tagName().compare("name",Qt::CaseInsensitive)==0 )//文件名节点
                {
                    nodeName = e.text();
                }
                if( e.tagName().compare("message",Qt::CaseInsensitive)==0 )//翻译节点
                {
                    QDomElement eLoc = e.firstChildElement("location");//文中位置
                    if( eLoc.isNull() )
                    {
                        //移除失效的节点
                        QDomElement temp = e; e = e.nextSiblingElement();
                        ele.removeChild(temp);
                        continue;
                    }
                    else
                    {
                        itemFile = new QTreeWidgetItem(0);//context name节点
                        itemFile->setText(0, nodeName);
                        itemRoot->addChild(itemFile);

                        QStringList stringLang;//语言翻译数据
                        while( !eLoc.isNull() )
                        {
                            //qDebug()<<eLoc.tagName()<<eLoc.text();
                            if( eLoc.tagName().compare("source",Qt::CaseInsensitive)==0 ||
                                eLoc.tagName().compare("translation",Qt::CaseInsensitive)==0 )
                            {
                                stringLang<<eLoc.text();
                            }
                            eLoc = eLoc.nextSiblingElement();
                        }
                        if( itemFile )//添加语言翻译
                        {
                            QTreeWidgetItem* itemLang = new QTreeWidgetItem(0);//context message location节点
                            itemLang->setText(0, stringLang.first());
                            itemLang->setToolTip(0, stringLang.last());
                            itemLang->setText(1, stringLang.last());
                            itemFile->addChild(itemLang);
                        }
                    }
                }//end message
                e = e.nextSiblingElement();
            }//
        }
        node = node.nextSibling();
    }

    //保存修改后的
    output_doc = doc;

    //langUrl += ".mod.ts";
    //QFile ofile(langUrl);
    //if( !ofile.open(QIODevice::WriteOnly | QIODevice::Truncate |QIODevice::Text) )
    //{
    //    return ;
    //}
    //QTextStream out(&ofile);
    //out.setCodec("UTF-8");
    //doc.save(out, 4, QDomNode::EncodingFromTextStream);
    //file.close();

    //展开所有项
    output_treewidget->expandAll();
}

