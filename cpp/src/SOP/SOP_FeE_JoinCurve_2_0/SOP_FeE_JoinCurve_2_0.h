
#ifndef __SOP_FeE_JoinCurve_2_0_h__
#define __SOP_FeE_JoinCurve_2_0_h__

#include <SOP/SOP_Node.h>
#include <UT/UT_StringHolder.h>

namespace SOP_FeE_JoinCurve_2_0_Namespace {
/// This is the SOP class definition.  It doesn't need to be in a separate
/// file like this.  This is just an example of a header file, in case
/// another file needs to reference something in here.
/// You shouldn't have to change anything in here except the name of the class.
class SOP_FeE_JoinCurve_2_0 : public SOP_Node
{
public:
    static PRM_Template *buildTemplates();
    static OP_Node *myConstructor(OP_Network *net, const char *name, OP_Operator *op)
    {
        OP_Node* newOp = new SOP_FeE_JoinCurve_2_0(net, name, op);
        //newOp->setColor(UT_Color(UT_ColorType::UT_RGB, 0.8, 0.5, 0.5));
        newOp->setNodeShape("tilted");
        //newOp->setUserData("nodeshape", "tilted", false);
        return newOp;
    }

    static const UT_StringHolder theSOPTypeName;
    
    const SOP_NodeVerb *cookVerb() const override;

protected:
    SOP_FeE_JoinCurve_2_0(OP_Network *net, const char *name, OP_Operator *op)
        : SOP_Node(net, name, op)
    {
        mySopFlags.setManagesDataIDs(true);
    }
    
    ~SOP_FeE_JoinCurve_2_0() override {}


    OP_ERROR cookMySop(OP_Context &context) override
    {
        return cookMyselfAsVerb(context);
    }
    const char* inputLabel(unsigned idx) const override
    {
        switch (idx)
        {
        case 0:     return "Curve";
        default:    return "Invalid Source";
        }
    }

    const char* outputLabel(unsigned idx) const override
    {
        switch (idx)
        {
        case 0:     return "Curve";
        default:    return "Invalid Output";
        }
    }

    int isRefInput(unsigned i) const override
    {
        return (i != 0);
    }


};
} // End SOP_FeE_JoinCurve_2_0_Namespace namespace

#endif
