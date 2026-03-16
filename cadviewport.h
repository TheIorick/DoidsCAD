#ifndef CADVIEWPORT_H
#define CADVIEWPORT_H

#include <QWidget>

class QLabel;

class CadViewport : public QWidget
{
    Q_OBJECT

public:
    explicit CadViewport(QWidget *parent = nullptr);

    void setSceneSummary(const QString &summary);

private:
    QLabel *m_titleLabel;
    QLabel *m_summaryLabel;
};

#endif // CADVIEWPORT_H
