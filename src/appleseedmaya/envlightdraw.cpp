
//
// This source file is part of appleseed.
// Visit http://appleseedhq.net/ for additional information and resources.
//
// This software is released under the MIT license.
//
// Copyright (c) 2017 Esteban Tovagliari, The appleseedhq Organization
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

// Interface header.
#include "appleseedmaya/envlightdraw.h"

// Standard headers.
#include <cmath>

// Maya headers.
#include <maya/MFloatArray.h>
#include <maya/MGL.h>
#include <maya/MIntArray.h>
#include <maya/MPointArray.h>

// appleseed.foundation headers.
#include "foundation/math/scalar.h"
#include "foundation/math/vector.h"

namespace asf = foundation;

namespace
{

bool g_initialized = false;

const size_t NumSubdivisions = 16;
MPointArray g_sphereVertices;
MFloatArray g_sphereUs;
MFloatArray g_sphereVs;

MPointArray g_logoVertices;
MIntArray g_logoStripSizes;

MBoundingBox g_boundingBox;

inline size_t vertexIndex(const size_t j, const size_t i)
{
    return j * NumSubdivisions + i;
}

void initializeGeom()
{
    if (g_initialized)
        return;

    // Sphere vertices.
    {
        const double radius = 1.0;

        for (size_t j = 0; j < NumSubdivisions; ++j)
        {
            const float t = asf::fit<size_t, double>(j, 0, NumSubdivisions, 0.0, 1.0);

            for (size_t i = 0; i < NumSubdivisions; ++i)
            {
                const float s = asf::fit<size_t, double>(i, 0, NumSubdivisions, 0.0, 1.0);

                const double theta = asf::Pi<double>() * t;
                const double phi = asf::TwoPi<double>() * s;
                asf::Vector3d p = radius * asf::Vector3d::make_unit_vector(theta, phi);
                g_sphereVertices.append(MPoint(p.x, p.y, p.z));
                g_sphereUs.append(s);
                g_sphereVs.append(t);
            }
        }
    }

    // Logo vertices
    {
        size_t i = 0;
        g_logoVertices.append(MPoint(4.99999602985, 6.24639415266, 0.0)); ++i;
        g_logoVertices.append(MPoint(4.84344021397, 5.95478169919, 0.0)); ++i;
        g_logoVertices.append(MPoint(4.77695111567, 5.74857723447, 0.0)); ++i;
        g_logoVertices.append(MPoint(4.75097549437, 5.54600980644, 0.0)); ++i;
        g_logoVertices.append(MPoint(4.78516283497, 5.37147218711, 0.0)); ++i;
        g_logoVertices.append(MPoint(4.87062698707, 5.26346158765, 0.0)); ++i;
        g_logoVertices.append(MPoint(4.99999602985, 5.21958256572, 0.0)); ++i;
        g_logoVertices.append(MPoint(5.12488923665, 5.25780446049, 0.0)); ++i;
        g_logoVertices.append(MPoint(5.21482963071, 5.37147218711, 0.0)); ++i;
        g_logoVertices.append(MPoint(5.24875492795, 5.54682314909, 0.0)); ++i;
        g_logoVertices.append(MPoint(5.22313636337, 5.74836664521, 0.0)); ++i;
        g_logoVertices.append(MPoint(5.15665391004, 5.95474029648, 0.0)); ++i;
        g_logoVertices.append(MPoint(4.99999643583, 6.24639415266, 0.0)); ++i;
        g_logoStripSizes.append(i);

        i = 0;
        g_logoVertices.append(MPoint(6.18539005392, 5.38516075070, 0.0)); ++i;
        g_logoVertices.append(MPoint(5.85967172213, 5.44394097566, 0.0)); ++i;
        g_logoVertices.append(MPoint(5.64301336096, 5.44345518195, 0.0)); ++i;
        g_logoVertices.append(MPoint(5.44233338012, 5.40556268809, 0.0)); ++i;
        g_logoVertices.append(MPoint(5.28690270915, 5.31911350450, 0.0)); ++i;
        g_logoVertices.append(MPoint(5.21058840012, 5.20445515493, 0.0)); ++i;
        g_logoVertices.append(MPoint(5.20883420315, 5.06785852031, 0.0)); ++i;
        g_logoVertices.append(MPoint(5.28377950862, 4.96088923718, 0.0)); ++i;
        g_logoVertices.append(MPoint(5.41967705096, 4.91047609857, 0.0)); ++i;
        g_logoVertices.append(MPoint(5.59692921938, 4.93239765080, 0.0)); ++i;
        g_logoVertices.append(MPoint(5.78069190285, 5.01904271899, 0.0)); ++i;
        g_logoVertices.append(MPoint(5.95642070078, 5.14604425488, 0.0)); ++i;
        g_logoVertices.append(MPoint(6.18539017937, 5.38516036459, 0.0)); ++i;
        g_logoStripSizes.append(i);

        i = 0;
        g_logoVertices.append(MPoint(5.73261531340, 3.99164828240, 0.0)); ++i;
        g_logoVertices.append(MPoint(5.68786612947, 4.31958891277, 0.0)); ++i;
        g_logoVertices.append(MPoint(5.62045299662, 4.52549314046, 0.0)); ++i;
        g_logoVertices.append(MPoint(5.52240156891, 4.70464171936, 0.0)); ++i;
        g_logoVertices.append(MPoint(5.39215279075, 4.82575080494, 0.0)); ++i;
        g_logoVertices.append(MPoint(5.25952380184, 4.86289864727, 0.0)); ++i;
        g_logoVertices.append(MPoint(5.12907060571, 4.82235630626, 0.0)); ++i;
        g_logoVertices.append(MPoint(5.05049614498, 4.71802375876, 0.0)); ++i;
        g_logoVertices.append(MPoint(5.04454505108, 4.57319899900, 0.0)); ++i;
        g_logoVertices.append(MPoint(5.12016761850, 4.41139630138, 0.0)); ++i;
        g_logoVertices.append(MPoint(5.25935776733, 4.26340240236, 0.0)); ++i;
        g_logoVertices.append(MPoint(5.43444659058, 4.13552001690, 0.0)); ++i;
        g_logoVertices.append(MPoint(5.73261498495, 3.99164804378, 0.0)); ++i;
        g_logoStripSizes.append(i);

        i = 0;
        g_logoVertices.append(MPoint(4.26739111044, 3.99164361521, 0.0)); ++i;
        g_logoVertices.append(MPoint(4.56545292559, 4.13554184611, 0.0)); ++i;
        g_logoVertices.append(MPoint(4.74044767937, 4.26328345096, 0.0)); ++i;
        g_logoVertices.append(MPoint(4.88052854524, 4.41189585562, 0.0)); ++i;
        g_logoVertices.append(MPoint(4.95546104432, 4.57319457044, 0.0)); ++i;
        g_logoVertices.append(MPoint(4.94980613030, 4.71081154918, 0.0)); ++i;
        g_logoVertices.append(MPoint(4.87093581813, 4.82235163907, 0.0)); ++i;
        g_logoVertices.append(MPoint(4.74742882528, 4.86483986171, 0.0)); ++i;
        g_logoVertices.append(MPoint(4.60785330464, 4.82574637638, 0.0)); ++i;
        g_logoVertices.append(MPoint(4.47733845320, 4.70382525755, 0.0)); ++i;
        g_logoVertices.append(MPoint(4.37960001261, 4.52571492964, 0.0)); ++i;
        g_logoVertices.append(MPoint(4.31208205850, 4.31967773297, 0.0)); ++i;
        g_logoVertices.append(MPoint(4.26739078200, 3.99164385384, 0.0)); ++i;
        g_logoStripSizes.append(i);

        i = 0;
        g_logoVertices.append(MPoint(3.81460749240, 5.38515319902, 0.0)); ++i;
        g_logoVertices.append(MPoint(4.04356900883, 5.14614656628, 0.0)); ++i;
        g_logoVertices.append(MPoint(4.21913484737, 5.01919099216, 0.0)); ++i;
        g_logoVertices.append(MPoint(4.40376101137, 4.93188993049, 0.0)); ++i;
        g_logoVertices.append(MPoint(4.58032062081, 4.91046893301, 0.0)); ++i;
        g_logoVertices.append(MPoint(4.70945468066, 4.95837306097, 0.0)); ++i;
        g_logoVertices.append(MPoint(4.79116334317, 5.06785096864, 0.0)); ++i;
        g_logoVertices.append(MPoint(4.79340628446, 5.19844268185, 0.0)); ++i;
        g_logoVertices.append(MPoint(4.71309496261, 5.31910633894, 0.0)); ++i;
        g_logoVertices.append(MPoint(4.55680978096, 5.40555764117, 0.0)); ++i;
        g_logoVertices.append(MPoint(4.35721395384, 5.44347330380, 0.0)); ++i;
        g_logoVertices.append(MPoint(4.14039674010, 5.44401769877, 0.0)); ++i;
        g_logoVertices.append(MPoint(3.81460761785, 5.38515358513, 0.0)); ++i;
        g_logoStripSizes.append(i);

        // Reduce logo size and bring it nearer the sphere.
        for (size_t i = 0, e = g_logoVertices.length(); i < e; ++i)
        {
            const float x = g_logoVertices[i].x;
            const float y = g_logoVertices[i].y;
            g_logoVertices[i].x = (x - 5.0) * 0.35 + 1.1;
            g_logoVertices[i].y = (y - 5.0) * 0.35 + 1.1;
        }
    }

    // Bounding box
    {
        g_boundingBox.clear();

        for (size_t i = 0, e = g_sphereVertices.length(); i < e; ++i)
            g_boundingBox.expand(g_sphereVertices[i]);

        for (size_t i = 0, e = g_logoVertices.length(); i < e; ++i)
            g_boundingBox.expand(g_logoVertices[i]);
    }

    g_initialized = true;
}

} // unnamed.

MBoundingBox sphereAndLogoBoundingBox(float size)
{
    initializeGeom();

    MBoundingBox boundingBox(
        g_boundingBox.min() * size,
        g_boundingBox.max() * size);
    return boundingBox;
}

void drawSphereWireframe(const float scale)
{
    initializeGeom();

    for (size_t j = 0; j < NumSubdivisions; ++j)
    {
        glBegin(GL_LINE_LOOP);
        for (size_t i = 0; i < NumSubdivisions; ++i)
        {
            const size_t index = vertexIndex(j, i);
            glVertex3f(
                g_sphereVertices[index].x * scale,
                g_sphereVertices[index].y * scale,
                g_sphereVertices[index].z * scale);
        }
        glEnd();
    }

    for (size_t j = 0; j < NumSubdivisions; ++j)
    {
        glBegin(GL_LINE_STRIP);
        for (size_t i = 0; i < NumSubdivisions; ++i)
        {
            const size_t index = vertexIndex(i, j);
            glVertex3f(
                g_sphereVertices[index].x * scale,
                g_sphereVertices[index].y * scale,
                g_sphereVertices[index].z * scale);
        }
        glEnd();
    }
}

void drawAppleseedLogo(const float scale)
{
    initializeGeom();

    size_t index = 0;
    for (size_t j = 0, je = g_logoStripSizes.length(); j < je; ++j)
    {
        glBegin(GL_LINE_STRIP);
        const size_t stripSize = g_logoStripSizes[j];
        for (size_t i = 0; i < stripSize; ++i)
        {
            glVertex3f(
                g_logoVertices[index].x * scale,
                g_logoVertices[index].y * scale,
                g_logoVertices[index].z * scale);
            ++index;
        }
        glEnd();
    }
}

