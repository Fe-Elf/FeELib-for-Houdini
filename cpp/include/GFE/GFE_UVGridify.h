
#pragma once

#ifndef __GFE_UVGridify_h__
#define __GFE_UVGridify_h__

#include <GFE/GFE_UVGridify.h>

#include <GFE/GeoFilter.h>

_GFEL_BEGIN
class UVGridify : public AttribFilter {

public:

    enum class RowColMethod
    {
        Uniform,
        Rows,
        Columns,
    };

    using AttribFilter::AttribFilter;



    void
        setComputeParm(
            const UVGridify::RowColMethod rowsOrColsNumMethod = UVGridify::RowColMethod::Uniform,
            const GA_Size rowsOrColsNum = 2,
            const bool reverseUVu = false,
            const bool reverseUVv = false,
            const bool uniScale = false,
            const exint subscribeRatio = 64,
            const exint minGrainSize = 64
        )
    {
        setHasComputed();
        this->rowsOrColsNumMethod = rowsOrColsNumMethod;
        this->rowsOrColsNum = rowsOrColsNum;
        this->reverseUVu = reverseUVu;
        this->reverseUVv = reverseUVv;
        this->uniScale = uniScale;
        this->subscribeRatio = subscribeRatio;
        this->minGrainSize = minGrainSize;
    }


private:



    virtual bool
        computeCore() override
    {
        if (getOutAttribArray().isEmpty())
            return false;
        
        if (groupParser.isEmpty())
            return true;
        
        setValidConstPosAttrib();
        
        const size_t size = getOutAttribArray().size();
        for (size_t i = 0; i < size; i++)
        {
            attrib = getOutAttribArray()[i];
            
            const gfe::AttribStorage attribStorage = gfe::Type::getAttribStorage(attrib);
            if (!gfe::Variant::isAttribStorageVF(attribStorage))
                continue;
            auto storageVariant = gfe::Variant::getAttribStorageVariantVF(attribStorage);
            auto isPointAttribVariant = gfe::Variant::getBoolVariant(attrib->getOwner() == GA_ATTRIB_POINT);
            std::visit([&] (auto storageVariant, auto isPointAttribVariant)
            {
                using type = typename gfe::Variant::getAttribStorage_t<storageVariant>;
                uvGridify<type, isPointAttribVariant>();
            }, storageVariant, isPointAttribVariant);
            
        }
        return true;
    }


    template<typename _Ty, bool isPointAttrib>
    void uvGridify()
    {
        using value_type = typename _Ty::value_type;
        using _Tpos = UT_Vector3T<value_type>;

        
        const GA_RWHandleT<_Ty> uv_h(attrib);
        const GA_ROHandleT<_Tpos> pos_h(posAttrib);
        
        pointRef = geo->getTopology().getPointRef();
        UTparallelFor(groupParser.getPrimitiveSplittableRange(), [this, &uv_h, &pos_h](const GA_SplittableRange& r)
        {
            value_type scale;
            bool scaleIdx = true;
            _Ty uv(gfe::Math::getZero<_Ty>());
            
            GA_Offset start, end;
            for (GA_Iterator it(r); it.blockAdvance(start, end); )
            {
                for (GA_Offset primoff = start; primoff < end; ++primoff)
                {
                    const GA_OffsetListRef& vertices = geo->getPrimitiveVertexList(primoff);
                    const GA_Size numvtx = vertices.size();
                    if (numvtx == 0)
                        continue;
                    GA_Size rows, cols;
                    switch (rowsOrColsNumMethod)
                    {
                    case UVGridify::RowColMethod::Uniform:
                        rows = GA_Size(std::ceil(numvtx / 4.0));
                        cols = (numvtx - rows - rows) / 2;
                        break;
                    case UVGridify::RowColMethod::Rows:
                        rows = rowsOrColsNum - 1;
                        cols = (numvtx - rows - rows) / 2;
                        break;
                    case UVGridify::RowColMethod::Columns:
                        cols = rowsOrColsNum - 1;
                        rows = (numvtx - cols - cols) / 2;
                        break;
                    default:
                        UT_ASSERT_MSG(0, "Unhandled rowsOrColsNumMethod");
                        break;
                    }
                    rows = std::max(rows, GA_Size(0));
                    cols = std::max(cols, GA_Size(0));


                    if (!uniScale)
                    {
                        if (numvtx > rows + rows + cols && numvtx > 2)
                        {
                            const _Tpos& pos0 = pos_h.get(pointRef->getLink(vertices[0]));
                            const _Tpos& pos1 = pos_h.get(pointRef->getLink(vertices[rows]));
                            const _Tpos& pos2 = pos_h.get(pointRef->getLink(vertices[rows + cols]));
                            //GA_Offset a = pointRef->getLink(vertices[rows + cols + rows]);
                            //GA_Offset b = geo->vertexPoint(a);
                            //const _Tpos& pos3 = pos_h.get(b);
                            const _Tpos& pos3 = pos_h.get(geo->vertexPoint(pointRef->getLink(vertices[rows + cols + rows])));
                            const value_type dist0 = pos0.distance(pos1);
                            const value_type dist1 = pos1.distance(pos2);
                            const value_type dist2 = pos2.distance(pos3);
                            const value_type dist3 = pos3.distance(pos0);

                            scale = (dist0 + dist2) / (dist1 + dist3);

                            const value_type tmpScale = dist0 + dist2;
                            scale = dist1 + dist3;

                            scaleIdx = tmpScale > scale;
                            scale = scaleIdx ? scale / tmpScale : tmpScale / scale;
                        }
                        else
                        {
                            scale = 1.0;
                        }
                    }

                    GA_Size vtxpnum = 0;
                    for (; vtxpnum < rows; vtxpnum++)
                    {
                        GA_Size tmpI = rows - (cols <= 0);
                        uv[0] = tmpI==0 ? 0 : fpreal(vtxpnum) / tmpI;
                        uv[1] = 1;

                        uvGridify<_Ty, isPointAttrib>(vertices, uv_h, uv, primoff, vtxpnum, scale, scaleIdx);
                    }
                    for (; vtxpnum < rows + cols; vtxpnum++)
                    {
                        GA_Size tmpI = cols - (rows == 0);
                        uv[0] = 1;
                        uv[1] = tmpI==0 ? 1 : 1 - (float(vtxpnum - rows) / tmpI);

                        uvGridify<_Ty, isPointAttrib>(vertices, uv_h, uv, primoff, vtxpnum, scale, scaleIdx);
                    }
                    const GA_Size numvtx_preCols = numvtx - cols;
                    for (; vtxpnum < numvtx_preCols; vtxpnum++)
                    {
                        GA_Size tmpI = vtxpnum - rows - cols;
                        uv[0] = tmpI == 0 ? 1 : (1 - float(tmpI) / (numvtx - rows - cols - cols - 1));
                        uv[1] = 0;

                        uvGridify<_Ty, isPointAttrib>(vertices, uv_h, uv, primoff, vtxpnum, scale, scaleIdx);
                    }
                    for (; vtxpnum < numvtx; vtxpnum++)
                    {
                        GA_Size tmpI = rows==0 ? numvtx - cols - 1 : cols;
                        uv[0] = 0;
                        uv[1] = tmpI==0 ? 1 : (1 - float(numvtx - vtxpnum - (rows == 0)) / tmpI);

                        uvGridify<_Ty, isPointAttrib>(vertices, uv_h, uv, primoff, vtxpnum, scale, scaleIdx);
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
    }
    

    template<typename _Ty, bool isPointAttrib>
    SYS_FORCE_INLINE void uvGridify(
        const GA_OffsetListRef& vertices,
        const GA_RWHandleT<_Ty>& uv_h,
        _Ty& uv,
        const GA_Offset primoff,
        const GA_Size vtxpnum,
        const typename _Ty::value_type scale,
        const bool scaleIdx
    )
    {
        if (reverseUVu)
            uv[0] = 1 - uv[0];
        
        if (reverseUVv)
            uv[1] = 1 - uv[1];
        
        if (!uniScale)
            uv[scaleIdx] *= scale;
        
        GA_Offset elemoff = vertices[vtxpnum];
        if (isPointAttrib)
            elemoff = pointRef->getLink(elemoff);
        uv_h.set(elemoff, uv);
    }



    
public:
    UVGridify::RowColMethod rowsOrColsNumMethod = UVGridify::RowColMethod::Uniform;
    GA_Size rowsOrColsNum = 2;
    bool reverseUVu = false;
    bool reverseUVv = false;
    bool uniScale = false;

private:
    GA_Attribute* attrib = nullptr;
    
    const GA_ATITopology* pointRef;
    
    exint subscribeRatio = 64;
    exint minGrainSize   = 1024;
    
}; // End of Class UVGridify
_GFEL_END
#endif
