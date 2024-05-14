
#ifndef __SOP_FeE_UVScaletoWorldSize_0_5_h__
#define __SOP_FeE_UVScaletoWorldSize_0_5_h__

#include <SOP/SOP_Node.h>
#include <UT/UT_StringHolder.h>

namespace SOP_FeE_UVScaletoWorldSize_0_5_Namespace {

class SOP_FeE_UVScaletoWorldSize_0_5 : public SOP_Node
{
public:
    static PRM_Template *buildTemplates();
    static OP_Node *myConstructor(OP_Network *net, const char *name, OP_Operator *op)
    {
        OP_Node* newOp = new SOP_FeE_UVScaletoWorldSize_0_5(net, name, op);
        //newOp->setColor(UT_Color(UT_ColorType::UT_RGB, 0.8, 0.5, 0.5));
        newOp->setNodeShape("tilted");
        //newOp->setUserData("nodeshape", "tilted", false);
        return newOp;
    }

    static const UT_StringHolder theSOPTypeName;
    
    const SOP_NodeVerb *cookVerb() const override;

protected:
    SOP_FeE_UVScaletoWorldSize_0_5(OP_Network *net, const char *name, OP_Operator *op)
        : SOP_Node(net, name, op)
    {
        mySopFlags.setManagesDataIDs(true);
    }
    
    ~SOP_FeE_UVScaletoWorldSize_0_5() override {}

    OP_ERROR cookMySop(OP_Context &context) override
    {
        return cookMyselfAsVerb(context);
    }

    const char* inputLabel(unsigned idx) const override
    {
        switch (idx)
        {
        case 0:     return "Geo with UV";
        default:    return "Invalid Source";
        }
    }

    const char* outputLabel(unsigned idx) const override
    {
        switch (idx)
        {
        case 0:     return "Geo";
        default:    return "Invalid Output";
        }
    }

    int isRefInput(unsigned i) const override
    {
        return (i != 0);
    }


};
} // End SOP_FeE_UVScaletoWorldSize_0_5_Namespace namespace

#endif
