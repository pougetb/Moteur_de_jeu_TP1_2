/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "mainwidget.h"

#include <QMouseEvent>

#include <math.h>

MainWidget::MainWidget(QWidget *parent) :
    QOpenGLWidget(parent),
    geometries(0),
    texture_grass(0),
    texture_rock(0),
    texture_snowrocks(0),
    height_map(0),
    angularSpeed(0)
{
}

MainWidget::~MainWidget()
{
    // Make sure the context is current when deleting the texture
    // and the buffers.
    makeCurrent();
    delete texture_grass;
    delete texture_rock;
    delete texture_snowrocks;
    delete height_map;
    delete geometries;
    doneCurrent();
}

//! [0]
void MainWidget::mousePressEvent(QMouseEvent *e)
{
    if(!auto_rotate) {
        // Save mouse press position
        mousePressPosition = QVector2D(e->localPos());
    }
}

void MainWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if(!auto_rotate) {
        // Mouse release position - mouse press position
        QVector2D diff = QVector2D(e->localPos()) - mousePressPosition;

        // Rotation axis is perpendicular to the mouse position difference
        // vector
        QVector3D n = QVector3D(diff.y(), diff.x(), 0.0).normalized();

        // Accelerate angular speed relative to the length of the mouse sweep
        qreal acc = diff.length() / 100.0;

        // Calculate new rotation axis as weighted sum
        rotationAxis = (rotationAxis * angularSpeed + n * acc).normalized();

        // Increase angular speed
        angularSpeed = acc;
    }
}
//! [0]

//! [1]
void MainWidget::timerEvent(QTimerEvent *)
{
    if(!auto_rotate) {
        // Decrease angular speed (friction)
        angularSpeed *= 0.99;

        // Stop rotation when speed goes below threshold
        if (angularSpeed < 0.01) {
            angularSpeed = 0.0;
        } else {
            // Update rotation
            rotation = QQuaternion::fromAxisAndAngle(rotationAxis, angularSpeed) * rotation;

            // Request an update
            update();
        }
    } else {
        rotation = QQuaternion::fromAxisAndAngle(rotationAxis, angularSpeed) * rotation;
        // Request an update
        update();

    }
}
//! [1]

void MainWidget::keyPressEvent(QKeyEvent *e){
    float step = 0.2f;
    if(e->key() == Qt::Key_Up){
        offset_y -= step;
    }
    if(e->key() == Qt::Key_Down){
        offset_y += step;
    }
    if(e->key() == Qt::Key_Left){
        offset_x += step;
    }
    if(e->key() == Qt::Key_Right){
        offset_x -= step;
    }
    if(e->key() == Qt::Key_PageUp){
        offset_z += step;
    }
    if(e->key() == Qt::Key_PageDown){
        offset_z -= step;
    }

    if(e->key() == Qt::Key_R){

        if(!auto_rotate) {
            auto_rotate = !auto_rotate;
            offset_x = 0;
            offset_y = 0;
            offset_z = -10;

            rotation = QQuaternion::fromEulerAngles(0.0, 0.0, 0.0);
            rotationAxis = QVector3D(0,1,1);
            angularSpeed = 0.5;
        } else {
            auto_rotate = !auto_rotate;
            rotation = QQuaternion::fromEulerAngles(0.0, 0.0, 0.0);
            angularSpeed = 0;
        }

    }

    update();
}


void MainWidget::initializeGL()
{
    initializeOpenGLFunctions();

    glClearColor(0, 0, 0, 1);

    initShaders();
    initTextures();

//! [2]
    // Enable depth buffer
    glEnable(GL_DEPTH_TEST);

    // Enable back face culling
    glEnable(GL_CULL_FACE);
//! [2]

    geometries = new GeometryEngine;

    // Use QBasicTimer because its faster than QTimer
    timer.start(12, this);
}

//! [3]
void MainWidget::initShaders()
{
    // Compile vertex shader
    if (!program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/vshader.glsl"))
        close();

    // Compile fragment shader
    if (!program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/fshader.glsl"))
        close();

    // Link shader pipeline
    if (!program.link())
        close();

    // Bind shader pipeline for use
    if (!program.bind())
        close();
}
//! [3]

//! [4]
void MainWidget::initTextures()
{
    texture_grass = new QOpenGLTexture(QImage(":/grass.png").mirrored());
    texture_grass->setMinificationFilter(QOpenGLTexture::Nearest);
    texture_grass->setMagnificationFilter(QOpenGLTexture::Linear);
    texture_grass->setWrapMode(QOpenGLTexture::Repeat);

    texture_rock = new QOpenGLTexture(QImage(":/rock.png").mirrored());
    texture_rock->setMinificationFilter(QOpenGLTexture::Nearest);
    texture_rock->setMagnificationFilter(QOpenGLTexture::Linear);
    texture_rock->setWrapMode(QOpenGLTexture::Repeat);

    texture_snowrocks = new QOpenGLTexture(QImage(":/snowrocks.png").mirrored());
    texture_snowrocks->setMinificationFilter(QOpenGLTexture::Nearest);
    texture_snowrocks->setMagnificationFilter(QOpenGLTexture::Linear);
    texture_snowrocks->setWrapMode(QOpenGLTexture::Repeat);

    height_map = new QOpenGLTexture(QImage(":/heightmap-1024x1024.png").mirrored());
    height_map->setMinificationFilter(QOpenGLTexture::Nearest);
    height_map->setMagnificationFilter(QOpenGLTexture::Linear);
    height_map->setWrapMode(QOpenGLTexture::Repeat);
}
//! [4]

//! [5]
void MainWidget::resizeGL(int w, int h)
{
    // Calculate aspect ratio
    qreal aspect = qreal(w) / qreal(h ? h : 1);

    // Set near plane to 3.0, far plane to 7.0, field of view 45 degrees
    const qreal zNear = 3.0, zFar = 50.0, fov = 45.0;

    // Reset projection
    projection.setToIdentity();

    // Set perspective projection
    projection.perspective(fov, aspect, zNear, zFar);
}
//! [5]

void MainWidget::paintGL()
{
    // Clear color and depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//! [6]
    // Calculate model view transformation
    QMatrix4x4 matrix;
    matrix.translate(0.0f + offset_x, 0.0f + offset_y, -5.0f + offset_z);
    matrix.rotate(rotation);
    matrix.lookAt(QVector3D(1,1,1), QVector3D(0,0,0), QVector3D(0,0,1));

    // Set modelview-projection matrix
    program.setUniformValue("mvp_matrix", projection * matrix);
//! [6]
//!
    texture_grass->bind(0);
    program.setUniformValue("texture_grass", 0);

    texture_rock->bind(1);
    program.setUniformValue("texture_rock",1);

    texture_snowrocks->bind(2);
    program.setUniformValue("texture_snowrocks", 2);

    height_map->bind(3);
    program.setUniformValue("height_map", 3);

    // Draw cube geometry
    geometries->drawCubeGeometry(&program);
}
