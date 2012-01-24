#include <stdexcept>
#include <boost/assign.hpp>

#include "ComponentRegistration.h"
#include "ComponentRegistry.h"
#include "CurrentLifetimeScope.h"
#include "DelegateActivator.h"
#include "IComponentRegistry.h"
#include "ILifetimeScope.h"
#include "LifetimeScope.h"
#include "ProvidedInstanceActivator.h"
#include "TypedService.h"
#include "Container.h"


namespace Hypodermic
{

    std::shared_ptr< IComponentRegistry > Container::componentRegistry()
    {
        return componentRegistry_;
    }

    std::shared_ptr< void > Container::resolveComponent(std::shared_ptr< IComponentRegistration > registration)
    {
        return rootLifetimeScope_->resolveComponent(registration);
    }

    std::shared_ptr< void > Container::getOrCreateInstance(std::shared_ptr< IComponentRegistration > registration)
    {
        return registration->activator()->activateInstance(shared_from_this());
    }

    void Container::initialize()
    {
        using namespace boost::assign;

        std::vector< std::shared_ptr< Service > > services = list_of(std::make_shared< TypedService >(typeid(ILifetimeScope)))
            (std::make_shared< TypedService >(typeid(IComponentContext)));

        componentRegistry_ = std::make_shared< ComponentRegistry >();

        componentRegistry_->addRegistration(std::make_shared< ComponentRegistration >(
            LifetimeScope::selfRegistrationId,
            std::make_shared< DelegateActivator< LifetimeScope > >(
                typeid(LifetimeScope),
                [](IComponentContext&) -> LifetimeScope*
                {
                    throw std::logic_error("Self registration cannot be activated");
                }),
            std::make_shared< CurrentLifetimeScope >(),
            InstanceSharing::Shared,
            InstanceOwnership::ExternallyOwned,
            services,
            std::unordered_map< std::type_index, std::shared_ptr< ITypeCaster > >()));

        services = list_of(std::make_shared< TypedService >(typeid(IContainer)));

        componentRegistry_->addRegistration(std::make_shared< ComponentRegistration >(
            LifetimeScope::selfRegistrationId,
            std::make_shared< ProvidedInstanceActivator< Container > >(shared_from_this()),
            std::make_shared< CurrentLifetimeScope >(),
            InstanceSharing::Shared,
            InstanceOwnership::ExternallyOwned,
            services,
            std::unordered_map< std::type_index, std::shared_ptr< ITypeCaster > >()));

        rootLifetimeScope_ = std::make_shared< LifetimeScope >(componentRegistry_);
        rootLifetimeScope_->initialize();
    }

} // namespace Hypodermic
