#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QTreeWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDomDocument>

//去除语言文件中的失效项
class MainWindow : public QWidget
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
public:
signals:
public slots:
    void onSlotChooseLangFileFunc();//选择语言文件
    void onSlotRecallRepeatTranslationFunc();//召回重复的翻译
    void onSlotTrimLangFileContextFunc();//输出过滤文件
    void onSlotSelectedRightItemFunc(QModelIndex);//选择右边对应翻译项
protected:
    void resizeEvent(QResizeEvent *);
private:
    void recallItemTranslation(QTreeWidgetItem* item);
    void readAndSave();
private:
    void prepareRepeatXml();//左边重复项
    void prepareOutputXml();//右边输出项
private:
    void initControl();
    void bindEvent();
private:
    QVBoxLayout* main_layout;
    QHBoxLayout* tool_layout;
    QVBoxLayout* repeat_title_layout;
    QVBoxLayout* output_title_layout;
    QHBoxLayout* content_layout;
    QHBoxLayout* save_layout;

    QLabel* url_label;//语言文件
    QLineEdit* url_linedit;
    QPushButton* url_button;//选择文件
    QPushButton* recall_button;//回收翻译
    QCheckBox* doall_chkbox;//全部替换
    QPushButton* trim_button;//清理操作
    QPushButton* cancel_button;

    QLabel* repeat_title_label;
    QTreeWidget* repeat_treewidget;//重复树
    QLabel* output_title_label;
    QTreeWidget* output_treewidget;//输出树

private:
    QDomDocument output_doc;
};

#endif // MAINWINDOW_H
