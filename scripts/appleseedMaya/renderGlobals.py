
#
# This source file is part of appleseed.
# Visit http://appleseedhq.net/ for additional information and resources.
#
# This software is released under the MIT license.
#
# Copyright (c) 2016-2017 Esteban Tovagliari, The appleseedhq Organization
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#

# Maya imports.
import maya.cmds as mc
import maya.mel as mel
import pymel.core as pm
import maya.OpenMaya as om

# appleseedMaya imports.
from logger import logger


def createGlobalNodes():
    if mc.objExists("appleseedRenderGlobals"):
        return

    sel = mc.ls(sl=True)
    mc.createNode(
        "appleseedRenderGlobals",
        name="appleseedRenderGlobals",
        shared=True,
        skipSelect=True)
    mc.lockNode("appleseedRenderGlobals")
    mc.select(sel, replace=True)
    logger.debug("Created appleseed render global node")

def createRenderTabsMelProcedures():
    pm.mel.source("createMayaSoftwareCommonGlobalsTab.mel")

    mel.eval('''
        global proc appleseedUpdateCommonTabProcedure()
        {
            updateMayaSoftwareCommonGlobalsTab();

            python("import appleseedMaya.renderGlobals");
            python("appleseedMaya.renderGlobals.postUpdateCommonTab()");
        }
        '''
    )
    mel.eval('''
        global proc appleseedCreateAppleseedTabProcedure()
        {
            python("import appleseedMaya.renderGlobals");
            python("appleseedMaya.renderGlobals.g_appleseedMainTab.create()");
        }
        '''
    )
    mel.eval('''
        global proc appleseedUpdateAppleseedTabProcedure()
        {
            python("import appleseedMaya.renderGlobals");
            python("appleseedMaya.renderGlobals.g_appleseedMainTab.update()");
        }
        '''
    )

def renderSettingsBuiltCallback(renderer):
    logger.debug("appleseedRenderSettingsBuilt called!")
    pm.renderer(
        "appleseed",
        edit=True,
        addGlobalsTab=(
            "Common",
            "createMayaSoftwareCommonGlobalsTab",
            "appleseedUpdateCommonTabProcedure"
            )
        )
    pm.renderer(
        "appleseed",
        edit=True,
        addGlobalsTab=(
            "Appleseed",
            "appleseedCreateAppleseedTabProcedure",
            "appleseedUpdateAppleseedTabProcedure"
            )
        )

g_nodeAddedCallbackID = None
g_nodeRemovedCallbackID = None
g_environmentLightsList = []

APPLESEED_ENVIRONMENT_LIGHTS = [
    "appleseedSkyDomeLight",
    "appleseedPhysicalSkyLight"]


def __nodeAdded(node, data):
    depNodeFn = om.MFnDependencyNode(node)
    nodeType = depNodeFn.typeName()

    if nodeType in APPLESEED_ENVIRONMENT_LIGHTS:
        logger.debug("Added or removed appleseed environment light")

        global g_environmentLightsList
        g_environmentLightsList.append(depNodeFn.name())
        g_appleseedMainTab.updateEnvLightControl()

def __nodeRemoved(node, data):
    depNodeFn = om.MFnDependencyNode(node)
    nodeType = depNodeFn.typeName()

    if nodeType in APPLESEED_ENVIRONMENT_LIGHTS:
        logger.debug("Removed appleseed environment light")

        global g_environmentLightsList
        g_environmentLightsList.remove(depNodeFn.name())
        g_appleseedMainTab.updateEnvLightControl()

def addRenderGlobalsScriptJobs():
    logger.debug("Adding render globals script jobs")

    global g_nodeAddedCallbackID
    assert g_nodeAddedCallbackID == None
    g_nodeAddedCallbackID = om.MDGMessage.addNodeAddedCallback(__nodeAdded)

    global g_nodeRemovedCallbackID
    assert g_nodeRemovedCallbackID == None
    g_nodeRemovedCallbackID = om.MDGMessage.addNodeRemovedCallback(__nodeRemoved)

    mc.scriptJob(
        attributeChange=[
            "defaultRenderGlobals.currentRenderer",
            "import appleseedMaya.renderGlobals; appleseedMaya.renderGlobals.currentRendererChanged()"
        ]
    )

def removeRenderGlobalsScriptJobs():
    global g_nodeAddedCallbackID
    assert g_nodeAddedCallbackID != None
    om.MMessage.removeCallback(g_nodeAddedCallbackID)
    g_nodeAddedCallbackID = None

    global g_nodeRemovedCallbackID
    assert g_nodeRemovedCallbackID != None
    om.MMessage.removeCallback(g_nodeRemovedCallbackID)
    g_nodeRemovedCallbackID = None

    logger.debug("Removed render globals script jobs")

def imageFormatChanged():
    logger.debug("imageFormatChanged called")

    # Since we only support two file formats atm., we can hardcode things.
    # 32 is the format code for png, 51 is custom image format.
    # We also update the extension attribute (used in the file names preview).
    newFormat = mc.getAttr("appleseedRenderGlobals.imageFormat")

    if newFormat == 0: # EXR
        mc.setAttr("defaultRenderGlobals.imageFormat", 51)
        mc.setAttr("defaultRenderGlobals.imfkey", "exr", type="string")
    elif newFormat == 1: # PNG
        mc.setAttr("defaultRenderGlobals.imageFormat", 32)
        mc.setAttr("defaultRenderGlobals.imfkey", "png", type="string")
    else:
        raise RuntimeError("Unknown render global image file format")

def currentRendererChanged():
    if mel.eval("currentRenderer()") != "appleseed":
        return

    logger.debug("currentRendererChanged called")

    # Make sure our render globals node exists.
    createGlobalNodes()

    # If the render globals window does not exist, create it.
    if not mc.window("unifiedRenderGlobalsWindow", exists=True):
        mel.eval("unifiedRenderGlobalsWindow")
        mc.window("unifiedRenderGlobalsWindow", edit=True, visible=False)

    # "Customize" the image formats menu.
    mc.setParent("unifiedRenderGlobalsWindow")
    mel.eval("setParentToCommonTab;")
    mc.setParent("imageFileOutputSW")
    mc.setParent("imageMenuMayaSW")
    mc.setParent("..")
    parent = mc.setParent(q=True)

    # Remove the menu callback and the menu items.
    mel.eval('optionMenuGrp -e -changeCommand "" imageMenuMayaSW;')
    items = mc.optionMenuGrp("imageMenuMayaSW", q=True, itemListLong=True)
    for item in items:
        mc.deleteUI(item)

    # Add the formats we support.
    menu = parent + "|imageMenuMayaSW|OptionMenu"
    mc.menuItem(parent=menu, label="OpenEXR (.exr)", data=0)
    mc.menuItem(parent=menu, label="PNG (.png)", data=1)

    # Connect the control to one internal attribute in our globals node
    # so that we can add a changed callback to it.
    mc.connectControl("imageMenuMayaSW", "appleseedRenderGlobals.imageFormat", index=1)
    mc.connectControl("imageMenuMayaSW", "appleseedRenderGlobals.imageFormat", index=2)

    # Add a callback when our internal attribute changes.
    # This callback gets the current value from our internal attribute and
    # uses it to update the original image format attribute (closing the circle.)
    mc.scriptJob(
        parent=parent,
        replacePrevious=True,
        attributeChange=[
            "appleseedRenderGlobals.imageFormat",
            "import appleseedMaya.renderGlobals; appleseedMaya.renderGlobals.imageFormatChanged()"]
    )

    # Update the image format controls now.
    imageFormatChanged()

def postUpdateCommonTab():
    imageFormatChanged()

class AppleseedRenderGlobalsMainTab(object):
    def __init__(self):
        self.__uis = {}

    def __addControl(self, ui, attrName, connectIndex=2):
        self.__uis[attrName] = ui
        attr = pm.Attribute("appleseedRenderGlobals." + attrName)
        pm.connectControl(ui, attr, index=connectIndex)

    def __limitBouncesChanged(self, value):
        self.__uis["bounces"].setEnable(value)
        self.__uis["specularBounces"].setEnable(value)
        self.__uis["glossyBounces"].setEnable(value)
        self.__uis["diffuseBounces"].setEnable(value)

    def __motionBlurChanged(self, value):
        self.__uis["mbCameraSamples"].setEnable(value)
        self.__uis["mbTransformSamples"].setEnable(value)
        self.__uis["mbDeformSamples"].setEnable(value)
        self.__uis["shutterOpen"].setEnable(value)
        self.__uis["shutterClose"].setEnable(value)

    def __environmentLightSelected(self, envLight):
        logger.debug("Environment light selected: %s" % envLight)

        connections = mc.listConnections(
            "appleseedRenderGlobals.envLight",
            plugs=True)
        if connections:
            mc.disconnectAttr(connections[0], "appleseedRenderGlobals.envLight")

        if envLight != "<none>":
            mc.connectAttr(
                envLight + ".globalsMessage",
                "appleseedRenderGlobals.envLight")

    def updateEnvLightControl(self):
        if "envLight" in self.__uis:
            logger.debug("Updating env lights menu")

            uiName = self.__uis["envLight"]

            # Return if the menu does not exist yet.
            if not pm.optionMenu(uiName, exists=True):
                return

            # Remove the callback.
            pm.optionMenu(uiName, edit=True, changeCommand="")

            # Delete the menu items.
            items = pm.optionMenu(uiName, query=True, itemListLong=True)
            for item in items:
                pm.deleteUI(item)

            connections = mc.listConnections("appleseedRenderGlobals.envLight")

            # Rebuild the menu.
            pm.menuItem(parent=uiName, label="<none>")
            for envLight in g_environmentLightsList:
                pm.menuItem(parent=uiName, label=envLight)

            # Update the currently selected item.
            if connections:
                node = connections[0]
                if mc.nodeType(node) == "transform":
                    shapes = mc.listRelatives(node, shapes=True)
                    assert shapes
                    node = shapes[0]
                    pm.optionMenu(uiName, edit=True, value=node)
            else:
                pm.optionMenu(uiName, edit=True, value="<none>")

            # Restore the callback.
            pm.optionMenu(uiName, edit=True, changeCommand=self.__environmentLightSelected)

    def create(self):
        # Create default render globals node if needed.
        createGlobalNodes()

        parentForm = pm.setParent(query=True)
        pm.setUITemplate("renderGlobalsTemplate", pushTemplate=True)
        pm.setUITemplate("attributeEditorTemplate", pushTemplate=True)

        columnWidth = 400

        with pm.scrollLayout("appleseedScrollLayout", horizontalScrollBarThickness=0):
            with pm.columnLayout("appleseedColumnLayout", adjustableColumn=True, width=columnWidth):
                with pm.frameLayout(label="Sampling", collapsable=True, collapse=False):
                    with pm.columnLayout("appleseedColumnLayout", adjustableColumn=True, width=columnWidth):
                        self.__addControl(
                            ui=pm.intFieldGrp(label="Pixel Samples", numberOfFields = 1),
                            attrName="samples")
                        self.__addControl(
                            ui=pm.intFieldGrp(label="Render Passes", numberOfFields = 1),
                            attrName="passes")
                        self.__addControl(
                            ui=pm.intFieldGrp(label="Tile Size", numberOfFields = 1),
                            attrName="tileSize")

                with pm.frameLayout(label="Shading", collapsable=True, collapse=False):
                    with pm.columnLayout("appleseedColumnLayout", adjustableColumn=True, width=columnWidth):
                        attr = pm.Attribute("appleseedRenderGlobals.diagnostics")
                        menuItems = [(i, v) for i, v in enumerate(attr.getEnums().keys())]
                        self.__addControl(
                            ui=pm.attrEnumOptionMenuGrp(label="Override Shaders", enumeratedItem=menuItems),
                            attrName = "diagnostics")

                with pm.frameLayout(label="Lighting", collapsable=True, collapse=False):
                    with pm.columnLayout("appleseedColumnLayout", adjustableColumn=True, width=columnWidth):
                        attr = pm.Attribute("appleseedRenderGlobals.lightingEngine")
                        menuItems = [(i, v) for i, v in enumerate(attr.getEnums().keys())]
                        self.__addControl(
                            ui=pm.attrEnumOptionMenuGrp(label="Lighting Engine", enumeratedItem=menuItems),
                            attrName = "lightingEngine")

                        with pm.frameLayout(label="Path Tracing", collapsable=True, collapse=False):
                            with pm.columnLayout("appleseedColumnLayout", adjustableColumn=True, width=columnWidth):
                                self.__addControl(
                                    ui=pm.checkBoxGrp(label="Limit Bounces", changeCommand=self.__limitBouncesChanged),
                                    attrName="limitBounces")
                                limitBounces = mc.getAttr("appleseedRenderGlobals.limitBounces")
                                self.__addControl(
                                    ui=pm.intFieldGrp(label="Global Bounces", numberOfFields = 1, enable=limitBounces),
                                    attrName="bounces")
                                self.__addControl(
                                    ui=pm.intFieldGrp(label="Diffuse Bounces", numberOfFields = 1, enable=limitBounces),
                                    attrName="diffuseBounces")
                                self.__addControl(
                                    ui=pm.intFieldGrp(label="Glossy Bounces", numberOfFields = 1, enable=limitBounces),
                                    attrName="glossyBounces")
                                self.__addControl(
                                    ui=pm.intFieldGrp(label="Specular Bounces", numberOfFields = 1, enable=limitBounces),
                                    attrName="specularBounces")
                                self.__addControl(
                                    ui=pm.floatFieldGrp(label="Light Samples", numberOfFields = 1),
                                    attrName="lightSamples")
                                self.__addControl(
                                    ui=pm.floatFieldGrp(label="Environment Samples", numberOfFields = 1),
                                    attrName="envSamples")
                                self.__addControl(
                                    ui=pm.checkBoxGrp(label="Caustics"),
                                    attrName="caustics")
                                self.__addControl(
                                    ui=pm.floatFieldGrp(label="Max Ray Intensity", numberOfFields = 1),
                                    attrName="maxRayIntensity")

                with pm.frameLayout(label="Environment", collapsable=True, collapse=False):
                    with pm.columnLayout("appleseedColumnLayout", adjustableColumn=True, width=columnWidth):
                        with pm.rowLayout("appleseedRowLayout", nc=3):
                            pm.text("Environment Light")
                            ui = pm.optionMenu(changeCommand=self.__environmentLightSelected)
                            pm.menuItem(label="<none>")

                            for envLight in g_environmentLightsList:
                                pm.menuItem(label=envLight)

                            self.__uis["envLight"] = ui
                            logger.debug("Created globals env light menu, name = %s" % ui)

                        self.__addControl(
                            ui=pm.checkBoxGrp(label="Background Emits Light"),
                            attrName="bgLight")

                with pm.frameLayout(label="Motion Blur", collapsable=True, collapse=True):
                    with pm.columnLayout("appleseedColumnLayout", adjustableColumn=True, width=columnWidth):
                        self.__addControl(
                            ui=pm.checkBoxGrp(label="Motion Blur", changeCommand=self.__motionBlurChanged),
                            attrName="motionBlur")

                        enableMotionBlur = mc.getAttr("appleseedRenderGlobals.motionBlur")
                        self.__addControl(
                            ui=pm.intFieldGrp(label="Camera Samples", numberOfFields = 1, enable=enableMotionBlur),
                            attrName="mbCameraSamples")
                        self.__addControl(
                            ui=pm.intFieldGrp(label="Transformation Samples", numberOfFields = 1, enable=enableMotionBlur),
                            attrName="mbTransformSamples")
                        self.__addControl(
                            ui=pm.intFieldGrp(label="Deformation Samples", numberOfFields = 1, enable=enableMotionBlur),
                            attrName="mbDeformSamples")
                        self.__addControl(
                            ui=pm.floatFieldGrp(label="Shutter Open", numberOfFields = 1, enable=enableMotionBlur),
                            attrName="shutterOpen")
                        self.__addControl(
                            ui=pm.floatFieldGrp(label="Shutter Close", numberOfFields = 1, enable=enableMotionBlur),
                            attrName="shutterClose")

                with pm.frameLayout(label="System", collapsable=True, collapse=False):
                    with pm.columnLayout("appleseedColumnLayout", adjustableColumn=True, width=columnWidth):
                        self.__addControl(
                            ui=pm.intFieldGrp(label="Threads", numberOfFields = 1),
                            attrName="threads")

        pm.setUITemplate("renderGlobalsTemplate", popTemplate=True)
        pm.setUITemplate("attributeEditorTemplate", popTemplate=True)
        pm.formLayout(
            parentForm,
            edit=True,
            attachForm=[
                ("appleseedScrollLayout", "top", 0),
                ("appleseedScrollLayout", "bottom", 0),
                ("appleseedScrollLayout", "left", 0),
                ("appleseedScrollLayout", "right", 0)])

        logger.debug("Created appleseed render global main tab")

        # Update the newly created tab.
        self.update()

    def update(self):
        assert mc.objExists("appleseedRenderGlobals")
        #self.updateEnvLightControl()

g_appleseedMainTab = AppleseedRenderGlobalsMainTab()
