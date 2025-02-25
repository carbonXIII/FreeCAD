/****************************************************************************
 *   Copyright (c) 2023 Zheng Lei (realthunder) <realthunder.dev@gmail.com> *
 *                                                                          *
 *   This file is part of the FreeCAD CAx development system.               *
 *                                                                          *
 *   This library is free software; you can redistribute it and/or          *
 *   modify it under the terms of the GNU Library General Public            *
 *   License as published by the Free Software Foundation; either           *
 *   version 2 of the License, or (at your option) any later version.       *
 *                                                                          *
 *   This library  is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *   GNU Library General Public License for more details.                   *
 *                                                                          *
 *   You should have received a copy of the GNU Library General Public      *
 *   License along with this library; see the file COPYING.LIB. If not,     *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,          *
 *   Suite 330, Boston, MA  02111-1307, USA                                 *
 *                                                                          *
 ****************************************************************************/

#ifndef MESH_PARAMS_H
#define MESH_PARAMS_H

#include <Mod/Mesh/MeshGlobal.h>

/*[[[cog
import MeshParams
MeshParams.declare()
]]]*/

// Auto generated code (Tools/params_utils.py:82)
#include <Base/Parameter.h>


// Auto generated code (Tools/params_utils.py:90)
namespace Mesh {
/** Convenient class to obtain Mesh related parameters

 * The parameters are under group "User parameter:BaseApp/Preferences/Mod/Mesh"
 *
 * This class is auto generated by Mod/Mesh/App/MeshParams.py. Modify that file
 * instead of this one, if you want to add any parameter. You need
 * to install Cog Python package for code generation:
 * @code
 *     pip install cogapp
 * @endcode
 *
 * Once modified, you can regenerate the header and the source file,
 * @code
 *     python3 -m cogapp -r Mod/Mesh/App/MeshParams.h Mod/Mesh/App/MeshParams.cpp
 * @endcode
 *
 * You can add a new parameter by adding lines in Mod/Mesh/App/MeshParams.py. Available
 * parameter types are 'Int, UInt, String, Bool, Float'. For example, to add
 * a new Int type parameter,
 * @code
 *     ParamInt(parameter_name, default_value, documentation, on_change=False)
 * @endcode
 *
 * If there is special handling on parameter change, pass in on_change=True.
 * And you need to provide a function implementation in Mod/Mesh/App/MeshParams.cpp with
 * the following signature.
 * @code
 *     void MeshParams:on<parameter_name>Changed()
 * @endcode
 */
class MeshExport MeshParams {
public:
    static ParameterGrp::handle getHandle();

    // Auto generated code (Tools/params_utils.py:139)
    //@{
    /// Accessor for parameter AsymptoteWidth
    static const std::string & getAsymptoteWidth();
    static const std::string & defaultAsymptoteWidth();
    static void removeAsymptoteWidth();
    static void setAsymptoteWidth(const std::string &v);
    static const char *docAsymptoteWidth();
    static void onAsymptoteWidthChanged();
    //@}

    // Auto generated code (Tools/params_utils.py:139)
    //@{
    /// Accessor for parameter AsymptoteHeight
    static const std::string & getAsymptoteHeight();
    static const std::string & defaultAsymptoteHeight();
    static void removeAsymptoteHeight();
    static void setAsymptoteHeight(const std::string &v);
    static const char *docAsymptoteHeight();
    static void onAsymptoteHeightChanged();
    //@}

    // Auto generated code (Tools/params_utils.py:139)
    //@{
    /// Accessor for parameter DefaultShapeType
    static const long & getDefaultShapeType();
    static const long & defaultDefaultShapeType();
    static void removeDefaultShapeType();
    static void setDefaultShapeType(const long &v);
    static const char *docDefaultShapeType();
    //@}

    // Auto generated code (Tools/params_utils.py:139)
    //@{
    /// Accessor for parameter MeshColor
    static const unsigned long & getMeshColor();
    static const unsigned long & defaultMeshColor();
    static void removeMeshColor();
    static void setMeshColor(const unsigned long &v);
    static const char *docMeshColor();
    //@}

    // Auto generated code (Tools/params_utils.py:139)
    //@{
    /// Accessor for parameter LineColor
    static const unsigned long & getLineColor();
    static const unsigned long & defaultLineColor();
    static void removeLineColor();
    static void setLineColor(const unsigned long &v);
    static const char *docLineColor();
    //@}

    // Auto generated code (Tools/params_utils.py:139)
    //@{
    /// Accessor for parameter MeshTransparency
    static const long & getMeshTransparency();
    static const long & defaultMeshTransparency();
    static void removeMeshTransparency();
    static void setMeshTransparency(const long &v);
    static const char *docMeshTransparency();
    //@}

    // Auto generated code (Tools/params_utils.py:139)
    //@{
    /// Accessor for parameter LineTransparency
    static const long & getLineTransparency();
    static const long & defaultLineTransparency();
    static void removeLineTransparency();
    static void setLineTransparency(const long &v);
    static const char *docLineTransparency();
    //@}

    // Auto generated code (Tools/params_utils.py:139)
    //@{
    /// Accessor for parameter TwoSideRendering
    static const bool & getTwoSideRendering();
    static const bool & defaultTwoSideRendering();
    static void removeTwoSideRendering();
    static void setTwoSideRendering(const bool &v);
    static const char *docTwoSideRendering();
    //@}

    // Auto generated code (Tools/params_utils.py:139)
    //@{
    /// Accessor for parameter VertexPerNormals
    static const bool & getVertexPerNormals();
    static const bool & defaultVertexPerNormals();
    static void removeVertexPerNormals();
    static void setVertexPerNormals(const bool &v);
    static const char *docVertexPerNormals();
    //@}

    // Auto generated code (Tools/params_utils.py:139)
    //@{
    /// Accessor for parameter CreaseAngle
    static const double & getCreaseAngle();
    static const double & defaultCreaseAngle();
    static void removeCreaseAngle();
    static void setCreaseAngle(const double &v);
    static const char *docCreaseAngle();
    //@}

    // Auto generated code (Tools/params_utils.py:139)
    //@{
    /// Accessor for parameter DisplayAliasFormatString
    static const std::string & getDisplayAliasFormatString();
    static const std::string & defaultDisplayAliasFormatString();
    static void removeDisplayAliasFormatString();
    static void setDisplayAliasFormatString(const std::string &v);
    static const char *docDisplayAliasFormatString();
    //@}

    // Auto generated code (Tools/params_utils.py:139)
    //@{
    /// Accessor for parameter ShowBoundingBox
    static const bool & getShowBoundingBox();
    static const bool & defaultShowBoundingBox();
    static void removeShowBoundingBox();
    static void setShowBoundingBox(const bool &v);
    static const char *docShowBoundingBox();
    //@}

    // Auto generated code (Tools/params_utils.py:139)
    //@{
    /// Accessor for parameter MaxDeviationExport
    static const double & getMaxDeviationExport();
    static const double & defaultMaxDeviationExport();
    static void removeMaxDeviationExport();
    static void setMaxDeviationExport(const double &v);
    static const char *docMaxDeviationExport();
    //@}

    // Auto generated code (Tools/params_utils.py:139)
    //@{
    /// Accessor for parameter RenderTriangleLimit
    static const long & getRenderTriangleLimit();
    static const long & defaultRenderTriangleLimit();
    static void removeRenderTriangleLimit();
    static void setRenderTriangleLimit(const long &v);
    static const char *docRenderTriangleLimit();
    //@}

    // Auto generated code (Tools/params_utils.py:139)
    //@{
    /// Accessor for parameter CheckNonManifoldPoints
    static const bool & getCheckNonManifoldPoints();
    static const bool & defaultCheckNonManifoldPoints();
    static void removeCheckNonManifoldPoints();
    static void setCheckNonManifoldPoints(const bool &v);
    static const char *docCheckNonManifoldPoints();
    //@}

    // Auto generated code (Tools/params_utils.py:139)
    //@{
    /// Accessor for parameter EnableFoldsCheck
    static const bool & getEnableFoldsCheck();
    static const bool & defaultEnableFoldsCheck();
    static void removeEnableFoldsCheck();
    static void setEnableFoldsCheck(const bool &v);
    static const char *docEnableFoldsCheck();
    //@}

    // Auto generated code (Tools/params_utils.py:139)
    //@{
    /// Accessor for parameter StrictlyDegenerated
    static const bool & getStrictlyDegenerated();
    static const bool & defaultStrictlyDegenerated();
    static void removeStrictlyDegenerated();
    static void setStrictlyDegenerated(const bool &v);
    static const char *docStrictlyDegenerated();
    //@}

    // Auto generated code (Tools/params_utils.py:139)
    //@{
    /// Accessor for parameter SubElementSelection
    static const bool & getSubElementSelection();
    static const bool & defaultSubElementSelection();
    static void removeSubElementSelection();
    static void setSubElementSelection(const bool &v);
    static const char *docSubElementSelection();
    //@}

// Auto generated code (Tools/params_utils.py:179)
}; // class MeshParams
} // namespace Mesh
//[[[end]]]
#endif // MESH_PARAMS_H
