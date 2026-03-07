#pragma once

#include "CoreGlobals.h"

#define RESULT_PARAM Z_Param__Result
#define RESULT_DECL void*const RESULT_PARAM

class FOutputDevice
{
public:
    FOutputDevice()
        : bSuppressEventTag(false)
        , bAutoEmitLineTerminator(true)
    {}

    FOutputDevice(FOutputDevice&&) = default;
    FOutputDevice(const FOutputDevice&) = default;
    FOutputDevice& operator=(FOutputDevice&&) = default;
    FOutputDevice& operator=(const FOutputDevice&) = default;

    virtual ~FOutputDevice() = default;
protected:
    bool bSuppressEventTag;
    bool bAutoEmitLineTerminator;
};

class FFrame : public FOutputDevice
{
public:
    UFunction* Node;
    UObject* Object;
    uint8_t* Code;
    uint8_t* Locals;

    UProperty* MostRecentProperty;
    uint8* MostRecentPropertyAddress;
    
    void*& GetPropertyChainForCompiledIn()
    {
        return *(void**)(__int64(this) + 0x80);
    }
    
    inline void Step(UObject* Context, RESULT_DECL)
    {
        static void (*StepOriginal)(FFrame*, UObject*, RESULT_DECL) = decltype(StepOriginal)(InSDKUtils::GetImageBase() + 0x1E83F60);
        StepOriginal(this, Context, RESULT_PARAM);
    }
    
    inline void StepExplicitProperty(void* Result, UProperty* Property)
    {
        static void (*StepExplicitPropertyOriginal)(FFrame*, void*, UProperty*) = decltype(StepExplicitPropertyOriginal)(InSDKUtils::GetImageBase() + 0x1E83F90);
        StepExplicitPropertyOriginal(this, Result, Property);
    }

    template<class TProperty = UProperty>
    void StepCompiledIn(void* Result);

    template<class TProperty = UProperty, typename TNativeType>
    TNativeType& StepCompiledInRef(void* TemporaryBuffer);
};

template<class TProperty>
void FFrame::StepCompiledIn(void* Result)
{
    if (Code)
    {
        Step(Object, Result);
    }
    else
    {
        TProperty* Property = (TProperty*)GetPropertyChainForCompiledIn();
        GetPropertyChainForCompiledIn() = Property->Next;
        StepExplicitProperty(Result, Property);
    }
}

template<class TProperty, typename TNativeType>
TNativeType& FFrame::StepCompiledInRef(void* TemporaryBuffer)
{
    MostRecentPropertyAddress = nullptr;

    if (Code)
    {
        Step(Object, TemporaryBuffer);
    }
    else
    {
        checkSlow(dynamic_cast<TProperty*>(GetPropertyChainForCompiledIn()) && dynamic_cast<UProperty*>(GetPropertyChainForCompiledIn()));
        TProperty* Property = (TProperty*)GetPropertyChainForCompiledIn();
        GetPropertyChainForCompiledIn() = Property->Next;
        StepExplicitProperty(TemporaryBuffer, Property);
    }

    return (MostRecentPropertyAddress != nullptr) ? *(TNativeType*)(MostRecentPropertyAddress) : *(TNativeType*)TemporaryBuffer;
}