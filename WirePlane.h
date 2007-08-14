#ifndef ROOT_TreeSearch_WirePlane
#define ROOT_TreeSearch_WirePlane

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// TreeSearch::WirePlane                                                     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "THaSubDetector.h"
#include <vector>

class TSeqCollection;

using std::vector;

namespace TreeSearch {

  class TimeToDistConv;

  class WirePlane : public THaSubDetector {

    enum EType { kUndefinedType = 0, kXPlane, kYPlane, kUPlane, kVPlane };

  public:
    WirePlane( const char* name, const char* description = "", 
	       THaDetectorBase* parent = NULL );
    virtual ~WirePlane();

    virtual void    Clear( Option_t* opt="" );
    virtual Int_t   Decode( const THaEvData& );
    virtual EStatus Init( const TDatime& date );
    virtual void    Print( Option_t* opt="" ) const;

    Int_t           Compare ( const TObject* obj ) const;
    Bool_t          IsSortable () const { return kTRUE; }

    EType           GetType() const { return fType; }
    Double_t        GetZ() const { return fOrigin.Z(); }
    WirePlane*      GetPartner() const { return fPartner; }
    void            SetPartner( WirePlane* p );

    Double_t        GetResolution() const { return fResolution; }
    TimeToDistConv* GetTTDConv() const { return fTTDConv; }

    TSeqCollection* GetHits() const { return fHits; }
    Int_t           GetNhits() const;

  protected:

    // Geometry, configuration

    EType     fType;         // Plane type (x,y,u,v)
    Double_t  fWireStart;    // Position of 1st wire (along wire coord) (m)
    Double_t  fWireSpacing;  // Wire spacing (assumed constant) (m)
    Double_t  fSinAngle;     // Sine of angle between dispersive direction (x)
                             //  and direction of decreasing wire number (rad)
    Double_t  fCosAngle;     // Cosine of wire angle (rad)
    WirePlane* fPartner;     //! Partner plane with staggered wires

    // Parameters, calibration, flags

    Double_t  fTDCRes;       // TDC Resolution ( s / channel)
    Double_t  fDriftVel;     // Drift velocity in the wire plane (m/s)
    Double_t  fResolution;   // Drift distance resolution (sigma) (m)

    TimeToDistConv* fTTDConv;   // Drift time->distance converter
    vector<double>  fTDCOffset; // [fNelem] TDC offsets for each wire

    // Event data, hits etc.

    TSeqCollection*   fHits;    // Hit data

    virtual Int_t ReadDatabase( const TDatime& date );
    virtual Int_t DefineVariables( EMode mode = kDefine );

    ClassDef(WirePlane,0)    // One MWDC wire plane
  };
}

///////////////////////////////////////////////////////////////////////////////

#endif
