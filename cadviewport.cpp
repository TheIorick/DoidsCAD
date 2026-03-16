#include "cadviewport.h"

#include <QFrame>
#include <QLabel>
#include <QVBoxLayout>

CadViewport::CadViewport(QWidget *parent)
    : QWidget(parent)
    , m_titleLabel(new QLabel(tr("3D Viewport"), this))
    , m_summaryLabel(new QLabel(this))
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(12);

    auto *frame = new QFrame(this);
    frame->setFrameShape(QFrame::StyledPanel);
    frame->setObjectName("viewportFrame");

    auto *frameLayout = new QVBoxLayout(frame);
    frameLayout->setContentsMargins(24, 24, 24, 24);
    frameLayout->setSpacing(8);

    m_titleLabel->setStyleSheet("font-size: 20px; font-weight: 600;");
    m_summaryLabel->setWordWrap(true);

    frameLayout->addWidget(m_titleLabel);
    frameLayout->addWidget(m_summaryLabel);
    frameLayout->addStretch();

    layout->addWidget(frame);

    setSceneSummary(tr("Viewport scaffold is ready. OpenCascade rendering and scene interaction will be connected next."));
}

void CadViewport::setSceneSummary(const QString &summary)
{
    m_summaryLabel->setText(summary);
}
