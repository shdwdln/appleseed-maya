
//
// This source file is part of appleseed.
// Visit http://appleseedhq.net/ for additional information and resources.
//
// This software is released under the MIT license.
//
// Copyright (c) 2016 Esteban Tovagliari, The appleseedhq Organization
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
#include "appleseedmaya/exporters/meshexporter.h"

// Standard headers.
#include <iostream>

// Boost headers.
#include "boost/filesystem/path.hpp"

// appleseed.maya headers.
#include "appleseedmaya/appleseedsession.h"
#include "appleseedmaya/exporters/exporterfactory.h"

namespace bfs = boost::filesystem;
namespace asf = foundation;
namespace asr = renderer;

void MeshExporter::registerExporter()
{
    NodeExporterFactory::registerDagNodeExporter("mesh", &MeshExporter::create);
}

DagNodeExporter *MeshExporter::create(const MDagPath& path, asr::Project& project)
{
    return new MeshExporter(path, project);
}

MeshExporter::MeshExporter(const MDagPath& path, asr::Project& project)
  : ShapeExporter(path, project)
{
}

void MeshExporter::createEntity()
{
    m_mesh = asr::MeshObjectFactory::create(appleseedName().asChar(), asr::ParamArray());
    m_mesh->push_material_slot("default");

    // Todo: create topology here...
}

void MeshExporter::exportShapeMotionStep(float time)
{
    // todo: fill geom info here...

    if(AppleseedSession::mode() == AppleseedSession::ExportSession)
    {
        // export mesh here...
        bfs::path projPath = project().search_paths().get_root_path();
        bfs::path geomPath = projPath / "_geometry";

        MurmurHash meshHash;
        // todo: compute mesh hash here.

        /*
        if(mesh file does not exist)
        {
            save mesh file here...
        }
        */

        // update the params dict.
    }
}

void MeshExporter::flushEntity()
{
    ShapeExporter::flushEntity();

    const MString objectInstanceName = appleseedName() + MString("_instance");

    asf::StringDictionary materials;
    //materials.insert("default", g_defaultMaterialName);

    asf::auto_release_ptr<asr::ObjectInstance> meshInstance;
    meshInstance = asr::ObjectInstanceFactory::create(
        objectInstanceName.asChar(),
        asr::ParamArray(),
        m_mesh->get_name(),
        m_transformSequence.get_earliest_transform(),
        materials,
        materials);

    std::cout << "Flushing mesh: " << m_mesh->get_name() << std::endl;
    mainAssembly().objects().insert(
        asf::auto_release_ptr<asr::Object>(m_mesh.release()));

    std::cout << "Flushing instance: " << objectInstanceName << std::endl;
    mainAssembly().object_instances().insert(meshInstance);
}
