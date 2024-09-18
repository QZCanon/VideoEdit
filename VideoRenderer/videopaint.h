#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QVector3D>
#include <QMatrix4x4>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QElapsedTimer>

class GLWidget;

class Renderer : public QObject, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit Renderer(GLWidget *w);

    void lockRenderer() { m_renderMutex.lock(); }
    void unlockRenderer() { m_renderMutex.unlock(); }
    QMutex *grabMutex() { return &m_grabMutex; }
    QWaitCondition *grabCond() { return &m_grabCond; }
    void prepareExit() { m_exiting = true; m_grabCond.wakeAll(); }

    void initGLBuffer();

signals:
    void contextWanted();

public slots:
    void render();

private:
    void paintQtLogo();
    void createGeometry();
    void quad(qreal x1, qreal y1, qreal x2, qreal y2, qreal x3, qreal y3, qreal x4, qreal y4);
    void extrude(qreal x1, qreal y1, qreal x2, qreal y2);
    void restoreContext();

    qreal m_fAngle;
    qreal m_fScale;
    QVector<QVector3D> m_vertices;
    QVector<QVector3D> m_normals;
    QOpenGLShaderProgram m_program;
    QOpenGLBuffer m_vbo;
    int m_vertexAttr;
    int m_normalAttr;
    int m_matrixUniform;
    GLWidget *m_glwidget;
    QMutex m_renderMutex;
    QElapsedTimer m_elapsed;
    QMutex m_grabMutex;
    QWaitCondition m_grabCond;
    bool m_exiting;
};

class GLWidget : public QOpenGLWidget
{
    Q_OBJECT

public:
    explicit GLWidget(QWidget *parent = nullptr);
    ~GLWidget();

protected:
    void paintEvent(QPaintEvent *) override { }
    void initializeGL() override;

signals:
    void renderRequested();

public slots:
    void grabContext();

private slots:
    void onAboutToCompose();
    void onFrameSwapped();
    void onAboutToResize();
    void onResized();

private:
    QThread *m_thread;
    Renderer *m_renderer;
};
