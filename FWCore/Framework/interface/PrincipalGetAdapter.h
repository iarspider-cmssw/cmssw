#ifndef FWCore_Framework_PrincipalGetAdapter_h
#define FWCore_Framework_PrincipalGetAdapter_h

// -*- C++ -*-
//

// Class  :     PrincipalGetAdapter
//
/**\class PrincipalGetAdapter PrincipalGetAdapter.h FWCore/Framework/interface/PrincipalGetAdapter.h

Description: This is the implementation for accessing EDProducts and 
inserting new EDProducts.

Usage:

Getting Data

The edm::PrincipalGetAdapter class provides many 'get*" methods for getting data
it contains.  

The primary method for getting data is to use getByLabel(). The labels are
the label of the module assigned in the configuration file and the 'product
instance label' (which can be omitted in the case the 'product instance label'
is the default value).  The C++ type of the product plus the two labels
uniquely identify a product in the PrincipalGetAdapter.

We use an event in the examples, but a run or a luminosity block can also
hold products.

\code
edm::Handle<AppleCollection> apples;
event.getByLabel("tree",apples);
\endcode

\code
edm::Handle<FruitCollection> fruits;
event.getByLabel("market", "apple", fruits);
\endcode


Putting Data

\code
  
//fill the collection
...
event.put(std::make_unique<AppleCollection>());
\endcode

\code

//fill the collection
...
event.put(std::make_unique<FruitCollection>());
\endcode


Getting a reference to a product before that product is put into the
event/lumiBlock/run.
NOTE: The edm::RefProd returned will not work until after the
edm::PrincipalGetAdapter has been committed (which happens after the
EDProducer::produce method has ended)
\code
auto pApples = std::make_unique<AppleCollection>();

edm::RefProd<AppleCollection> refApples = event.getRefBeforePut<AppleCollection>();

//do loop and fill collection
for(unsigned int index = 0; .....) {
....
apples->push_back(Apple(...));
  
//create an edm::Ref to the new object
edm::Ref<AppleCollection> ref(refApples, index);
....
}
\endcode

*/
/*----------------------------------------------------------------------

----------------------------------------------------------------------*/
#include <cassert>
#include <typeinfo>
#include <string>
#include <vector>
#include <type_traits>

#include "DataFormats/Common/interface/EDProductfwd.h"
#include "DataFormats/Provenance/interface/ProvenanceFwd.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"

#include "DataFormats/Common/interface/traits.h"

#include "DataFormats/Common/interface/BasicHandle.h"

#include "DataFormats/Common/interface/ConvertHandle.h"

#include "DataFormats/Common/interface/Handle.h"

#include "DataFormats/Common/interface/Wrapper.h"

#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/Utilities/interface/EDGetToken.h"
#include "FWCore/Utilities/interface/EDPutToken.h"
#include "FWCore/Utilities/interface/ProductKindOfType.h"
#include "FWCore/Utilities/interface/ProductLabels.h"
#include "FWCore/Utilities/interface/propagate_const.h"
#include "FWCore/Utilities/interface/Transition.h"

namespace edm {

  class ModuleCallingContext;
  class SharedResourcesAcquirer;
  class ProducerBase;

  namespace principal_get_adapter_detail {
    void throwOnPutOfNullProduct(char const* principalType,
                                 TypeID const& productType,
                                 std::string const& productInstanceName);
    void throwOnPutOfUninitializedToken(char const* principalType, std::type_info const& productType);
    void throwOnPutOfWrongType(std::type_info const& wrongType, TypeID const& rightType);
    void throwOnPrematureRead(char const* principalType,
                              TypeID const& productType,
                              std::string const& moduleLabel,
                              std::string const& productInstanceName);

    void throwOnPrematureRead(char const* principalType, TypeID const& productType, EDGetToken);

  }  // namespace principal_get_adapter_detail
  class PrincipalGetAdapter {
  public:
    PrincipalGetAdapter(Principal const& pcpl, ModuleDescription const& md, bool isComplete);

    ~PrincipalGetAdapter();

    PrincipalGetAdapter(PrincipalGetAdapter const&) = delete;             // Disallow copying and moving
    PrincipalGetAdapter& operator=(PrincipalGetAdapter const&) = delete;  // Disallow copying and moving

    //size_t size() const;

    void setConsumer(EDConsumerBase const* iConsumer) { consumer_ = iConsumer; }
    EDConsumerBase const* getConsumer() const { return consumer_; }

    void setSharedResourcesAcquirer(SharedResourcesAcquirer* iSra) { resourcesAcquirer_ = iSra; }
    SharedResourcesAcquirer* getSharedResourcesAcquirer() const { return resourcesAcquirer_; }

    void setProducer(ProducerBase const* iProd) { prodBase_ = iProd; }

    size_t numberOfProductsConsumed() const;

    bool isComplete() const { return isComplete_; }

    template <typename PROD>
    bool checkIfComplete() const;

    Transition transition() const;

    ProcessHistory const& processHistory() const;

    Principal const& principal() const { return principal_; }

    ProductDescription const& getProductDescription(TypeID const& type, std::string const& productInstanceName) const;

    EDPutToken::value_type getPutTokenIndex(TypeID const& type, std::string const& productInstanceName) const;

    TypeID const& getTypeIDForPutTokenIndex(EDPutToken::value_type index) const;
    std::string const& productInstanceLabel(EDPutToken) const;
    typedef std::vector<BasicHandle> BasicHandleVec;

    ProductDescription const& getProductDescription(unsigned int iPutTokenIndex) const;
    ProductID const& getProductID(unsigned int iPutTokenIndex) const;
    ModuleDescription const& moduleDescription() const { return md_; }

    std::vector<edm::ProductResolverIndex> const& putTokenIndexToProductResolverIndex() const;

    //uses the EDPutToken index
    std::vector<bool> const& recordProvenanceList() const;
    //------------------------------------------------------------
    // Protected functions.
    //

    // The following 'get' functions serve to isolate the PrincipalGetAdapter class
    // from the Principal class.

    BasicHandle getByLabel_(TypeID const& tid, InputTag const& tag, ModuleCallingContext const* mcc) const;

    BasicHandle getByLabel_(TypeID const& tid,
                            std::string const& label,
                            std::string const& instance,
                            std::string const& process,
                            ModuleCallingContext const* mcc) const;

    BasicHandle getByToken_(TypeID const& id,
                            KindOfType kindOfType,
                            EDGetToken token,
                            ModuleCallingContext const* mcc) const;

    BasicHandle getMatchingSequenceByLabel_(TypeID const& typeID,
                                            InputTag const& tag,
                                            ModuleCallingContext const* mcc) const;

    BasicHandle getMatchingSequenceByLabel_(TypeID const& typeID,
                                            std::string const& label,
                                            std::string const& instance,
                                            std::string const& process,
                                            ModuleCallingContext const* mcc) const;

    // Also isolates the PrincipalGetAdapter class
    // from the Principal class.
    EDProductGetter const* prodGetter() const;

    void labelsForToken(EDGetToken const& iToken, ProductLabels& oLabels) const;

    unsigned int processBlockIndex(std::string const& processName) const;

  private:
    template <typename T>
    static constexpr bool hasMergeProductFunction() {
      if constexpr (requires(T& a, T const& b) { a.mergeProduct(b); }) {
        return true;
      }
      return false;
    }
    // Is this an Event, a LuminosityBlock, or a Run.
    BranchType const& branchType() const;

    BasicHandle makeFailToGetException(KindOfType, TypeID const&, EDGetToken) const;

    void throwAmbiguousException(TypeID const& productType, EDGetToken token) const;

    void throwUnregisteredPutException(TypeID const& type, std::string const& productInstanceLabel) const;

  private:
    //------------------------------------------------------------
    // Data members
    //

    // Each PrincipalGetAdapter must have an associated Principal, used as the
    // source of all 'gets' and the target of 'puts'.
    Principal const& principal_;

    // Each PrincipalGetAdapter must have a description of the module executing the
    // "transaction" which the PrincipalGetAdapter represents.
    ModuleDescription const& md_;

    EDConsumerBase const* consumer_;
    SharedResourcesAcquirer* resourcesAcquirer_;  // We do not use propagate_const because the acquirer is itself mutable.
    ProducerBase const* prodBase_ = nullptr;
    bool isComplete_;
  };

  template <typename PROD>
  inline std::ostream& operator<<(std::ostream& os, Handle<PROD> const& h) {
    os << h.product() << " " << h.provenance() << " " << h.id();
    return os;
  }

  //------------------------------------------------------------
  // Metafunction support for compile-time selection of code used in
  // PrincipalGetAdapter::put member template.
  //

  namespace detail {
    template <typename T>
    void do_post_insert_if_available(T& iProduct) {
      if constexpr (not std::derived_from<T, DoNotSortUponInsertion> and requires(T& p) { p.post_insert(); }) {
        iProduct.post_insert();
      }
    }
  }  // namespace detail

  // Implementation of  PrincipalGetAdapter  member templates. See  PrincipalGetAdapter.cc for the
  // implementation of non-template members.
  //

  template <typename PROD>
  inline bool PrincipalGetAdapter::checkIfComplete() const {
    return isComplete() || !hasMergeProductFunction<PROD>();
  }

}  // namespace edm
#endif
