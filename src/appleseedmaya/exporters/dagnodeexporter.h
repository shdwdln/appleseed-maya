
//
// This source file is part of appleseed.
// Visit http://appleseedhq.net/ for additional information and resources.
//
// This software is released under the MIT license.
//
// Copyright (c) 2016-2017 Esteban Tovagliari, The appleseedhq Organization
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

#ifndef APPLESEED_MAYA_EXPORTERS_DAGNODEEXPORTER_H
#define APPLESEED_MAYA_EXPORTERS_DAGNODEEXPORTER_H

// Forward declaration header.
#include "dagnodeexporterfwd.h"

// Maya headers.
#include <maya/MDagPath.h>
#include <maya/MMatrix.h>
#include <maya/MObject.h>
#include <maya/MObjectArray.h>
#include <maya/MString.h>

// appleseed.foundation headers.
#include "foundation/math/matrix.h"

// appleseed.renderer headers.
#include "renderer/api/utility.h"

// appleseed.maya headers.
#include "appleseedmaya/appleseedsession.h"
#include "appleseedmaya/utils.h"

// Forward declarations.
namespace renderer { class Assembly; }
namespace renderer { class Project; }
namespace renderer { class Scene; }
class MotionBlurTimes;

class DagNodeExporter
  : public NonCopyable
{
  public:

    // Destructor.
    virtual ~DagNodeExporter();

    // Return the name of the entity in the appleseed project.
    MString appleseedName() const;

    // Return true if the entity created by this exporter can be motion blurred.
    virtual bool supportsMotionBlur() const;

    // Create any extra exporter needed by this exporter (shading engines, ...).
    virtual void createExporters(const AppleseedSession::Services& services);

    // Create appleseed entities.
    virtual void createEntities(
        const AppleseedSession::Options&            options,
        const AppleseedSession::MotionBlurTimes&    motionBlurTimes) = 0;

    // Motion blur.
    virtual void collectMotionBlurSteps(MotionBlurTimes& motionTimes) const;
    virtual void exportCameraMotionStep(float time);
    virtual void exportTransformMotionStep(float time);
    virtual void exportShapeMotionStep(float time);

    // Flush entities to the renderer.
    virtual void flushEntities() = 0;

  protected:

    DagNodeExporter(
      const MDagPath&                               path,
      renderer::Project&                            project,
      AppleseedSession::SessionMode                 sessionMode);

    // Return the Maya dependency node.
    MObject node() const;

    // Return the Maya dag path.
    const MDagPath& dagPath() const;

    // Return the session mode.
    AppleseedSession::SessionMode sessionMode() const;

    // Return a reference to the appleseed project.
    renderer::Project& project();

    // Return a reference to the appleseed scene.
    renderer::Scene& scene();

    // Return a reference to the appleseed main assembly.
    renderer::Assembly& mainAssembly();

    // Convert a Maya matrix to an appleseed matrix.
    foundation::Matrix4d convert(const MMatrix& m) const;

    void visibilityAttributesToParams(renderer::ParamArray& params);

    static bool isObjectRenderable(const MDagPath& path);
    static bool areObjectAndParentsRenderable(const MDagPath& path);

    static bool isAnimated(MObject object, bool checkParent=false);

  private:

    MDagPath                      m_path;
    AppleseedSession::SessionMode m_sessionMode;
    renderer::Project&            m_project;
    renderer::Scene&              m_scene;
    renderer::Assembly&           m_mainAssembly;
};

#endif  // !APPLESEED_MAYA_EXPORTERS_DAGNODEEXPORTER_H
