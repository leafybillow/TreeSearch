#ifndef ROOT_TreeSearch_Road
#define ROOT_TreeSearch_Road

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// TreeSearch::Road                                                          //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "Hit.h"
#include "Node.h"
#include "TVector2.h"
#include <set>
#include <utility>
#include <vector>
#include <list>
#include <functional>
#include <cassert>
#include <cstring>

class THaTrack;

namespace TreeSearch {

  class Projection;
  class BuildInfo_t;    // Defined in implementation

  class Road : public TObject {

  public:
    //_________________________________________________________________________
    // Coordinates of hit positions, for track fitting
    struct Point {
      Point() : x(0), hit(0) {}
      Point( Double_t _x, Double_t _z, Hit* _hit ) 
	: x(_x), z(_z), hit(_hit) { assert(hit); }
      virtual ~Point() {}
      Double_t res() const { return hit->GetResolution(); }
      Double_t x;    // Selected x coordinates
      Double_t z;    // z coordinate
      Hit*     hit;  // Associated hit (stored in WirePlane)
      ClassDef(Point,0)
    };

    typedef std::vector<Road::Point*>  Pvec_t;
    typedef std::list<const Node_t*>   NodeList_t;

    //_________________________________________________________________________
    // Fit results
    struct FitResult {
      Double_t fPos, fSlope, fChi2, fV[3];
      Pvec_t   fFitCoordinates;

      FitResult( Double_t pos, Double_t slope, Double_t chi2, 
		 const Double_t* cov )
	: fPos(pos), fSlope(slope), fChi2(chi2)
      { assert(cov); memcpy(fV, cov, 3*sizeof(Double_t)); }
      FitResult() {}
      // Copying and assignment do not transfer the fFitCoordinates!
      FitResult( const FitResult& orig )
	: fPos(orig.fPos), fSlope(orig.fSlope), fChi2(orig.fChi2)
      { memcpy(fV, orig.fV, 3*sizeof(Double_t)); }
      FitResult& operator=( const FitResult& rhs ) {
	if( this != &rhs ) {
	  fPos = rhs.fPos; fSlope = rhs.fSlope; fChi2 = rhs.fChi2;
	  memcpy(fV, rhs.fV, 3*sizeof(Double_t));
	}
	return *this;
      }
      void Set( Double_t pos, Double_t slope, Double_t chi2, 
		const Double_t* cov ) {
	assert(cov);
	fPos = pos; fSlope = slope; fChi2 = chi2;
	memcpy(fV, cov, 3*sizeof(Double_t));
      }

      // Sort fit results by ascending chi2
      bool operator<( const FitResult& rhs ) const 
      { return ( fChi2 < rhs.fChi2 ); }
      const Pvec_t& GetPoints() const  { return fFitCoordinates; }

      struct Chi2IsLess
	: public std::binary_function< FitResult*, FitResult*, bool >
      {
	bool operator() ( const FitResult* a, const FitResult* b ) const
	{ assert(a&&b); return ( a->fChi2 < b->fChi2 ); }
      };
    };

    //_________________________________________________________________________
    // For global variable access/event display
    friend class Corners;
    class Corners : public TObject {
    public:
      explicit Corners( Road* rd ) 
	: fXLL(rd->fCornerX[0]), fXLR(rd->fCornerX[1]), fZL(rd->fZL), 
	  fXUL(rd->fCornerX[3]), fXUR(rd->fCornerX[2]), fZU(rd->fZU) {} 
      Corners() {}  // For ROOT RTTI
      virtual ~Corners() {}
    private:
      Double_t fXLL;  // Lower left corner x coord
      Double_t fXLR;  // Lower right corner x coord
      Double_t fZL;   // Lower edge z coord
      Double_t fXUL;  // Upper left corner x coord
      Double_t fXUR;  // Upper right corner x coord
      Double_t fZU;   // Upper edge z coord
      ClassDef(Corners,0)
    };

    //_________________________________________________________________________
    explicit Road( const Projection* proj );
    Road( const Node_t& nd, const Projection* proj );
    Road() : fProjection(0), fTrack(0), fBuild(0) {} // For internal ROOT use
    Road( const Road& );
    Road& operator=( const Road& );
    virtual ~Road();

    Bool_t         Add( const Node_t& nd );
    virtual Int_t  Compare( const TObject* obj ) const;
    void           Finish();
    Bool_t         Fit();
    Double_t       GetChi2() const { return fChi2; }
    FitResult*     GetFitResult() const;
    UInt_t         GetNfits() const { return (UInt_t)fFitData.size(); }
    const Pvec_t&  GetPoints() const;
    Double_t       GetPos() const { return fPos; }
    Double_t       GetPos( Double_t z ) const { return fPos + z*fSlope; }
    Double_t       GetPosErrsq( Double_t z ) const;
    const Projection* GetProjection() const { return fProjection; }
    Double_t       GetSlope() const { return fSlope; }
    THaTrack*      GetTrack() const { return fTrack; }
    Bool_t         Include ( const Road* other );
    TVector2       Intersect( const Road* other, Double_t z ) const;
    Bool_t         IsGood() const { return fGood; }
    virtual Bool_t IsSortable () const { return kTRUE; }
    Bool_t         IsVoid() const { return !fGood; }
    virtual void   Print( Option_t* opt="" ) const;
    void           SetTrack( THaTrack* track ) { fTrack = track; }
    void           Void() { fGood = false; }

#ifdef VERBOSE
    const NodeList_t& GetPatterns() const { return fPatterns; }
#endif

  protected:

    const Projection*  fProjection; //! Projection that this Road belongs to

    Double_t           fCornerX[5]; // x positions of corners
    Double_t           fZL, fZU;    // z -/+ eps of first/last plane 

    // Best fit results (copy of fFitData.begin() for global variable access)
    // Caution: must be 6 Double_t in same order as in FitResult
    Double_t     fPos;       // Track origin
    Double_t     fSlope;     // Track slope
    Double_t     fChi2;      // Chi2 of fit
    Double_t     fV[3];      // Covariance matrix of param (V11, V12=V21, V22)

    UInt_t       fDof;       // Degrees of freedom of fit (nhits-2)
    Bool_t       fGood;      // Road successfully built and fit
    THaTrack*    fTrack;     // The lowest-chi2 3D track using this road

    NodeList_t   fPatterns;  // Patterns in this road
    Hset_t       fHits;      // All hits linked to the patterns
    
    std::vector<Pvec_t>      fPoints;    // Hit pos within road
    std::vector<FitResult*>  fFitData;   // Good fit results, sorted by chi2

    BuildInfo_t* fBuild;     //! Working data for building

    Bool_t   CheckMatch( const Hset_t& hits ) const;
    Bool_t   CollectCoordinates();
    Bool_t   IsInRange( const NodeDescriptor& nd ) const;

  private:
    void     CopyPointData( const Road& orig );

    ClassDef(Road,1)  // Region containing track candidate hits and fit results
  };

  //___________________________________________________________________________
  inline
  Int_t Road::Compare( const TObject* obj ) const 
  {
    // Used for sorting Roads in a TClonesArray or similar.
    // A Road is "less than" another if the chi2 of its best fit is smaller.
    // Returns -1 if this is smaller than rhs, 0 if equal, +1 if greater.

    // Require identical classes of objects
    assert( obj && IsA() == obj->IsA() );

    //TODO: take fDof into account & compare statistical significance
    if( fChi2 < static_cast<const Road*>(obj)->fChi2 ) return -1;
    if( fChi2 > static_cast<const Road*>(obj)->fChi2 ) return  1;
    return 0;
  }

  //---------------------------------------------------------------------------
  inline
  const TreeSearch::Road::Pvec_t& Road::GetPoints() const
  {
    // Return points used by the best fit
    assert( !fFitData.empty() );
    return fFitData.front()->GetPoints();
  }

  //---------------------------------------------------------------------------
  inline
  Road::FitResult* Road::GetFitResult() const
  {
    // Return results of the best fit
    assert( !fFitData.empty() );
    return fFitData.front();
  }

  //---------------------------------------------------------------------------
  inline
  Double_t Road::GetPosErrsq( Double_t z ) const
  {
    // Return square of uncertainty in x = a1+z2*z for best fit (in m^2)
    
    return fV[0] + 2.0*fV[1]*z + fV[2]*z*z;
  }

///////////////////////////////////////////////////////////////////////////////

} // end namespace TreeSearch


#endif
