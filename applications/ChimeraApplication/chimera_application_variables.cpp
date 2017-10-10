#include "chimera_application_variables.h"

namespace Kratos
{

KRATOS_CREATE_3D_VARIABLE_WITH_COMPONENTS(CHIM_NEUMANN_COND);

KRATOS_CREATE_VARIABLE(MpcDataPointerVectorType, MPC_DATA_CONTAINER)

//KRATOS_CREATE_VARIABLE(bool, IS_WEAK);

KRATOS_CREATE_VARIABLE(double, BOUNDARY_NODE);
KRATOS_CREATE_VARIABLE(double, FLUX);
KRATOS_CREATE_3D_VARIABLE_WITH_COMPONENTS(TRACTION);

}