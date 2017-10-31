//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:         BSD License
//                   Kratos default license: kratos/license.txt
//
//  Main authors:    Jordi Cotela
//

#include "dss.h"
#include "fluid_element.h"
#include "includes/cfd_variables.h"
#include "includes/checks.h"

namespace Kratos
{

///////////////////////////////////////////////////////////////////////////////////////////////////
// Class template instantiation

template class DSS< DSSData2D3N >;
template class DSS< DSSData3D4N >;

///////////////////////////////////////////////////////////////////////////////////////////////////
// Life cycle

template< class TElementData >
DSS<TElementData>::DSS(IndexType NewId):
    FluidElement<TElementData>(NewId)
{}

template< class TElementData >
DSS<TElementData>::DSS(IndexType NewId, const NodesArrayType& ThisNodes):
    FluidElement<TElementData>(NewId,ThisNodes)
{}


template< class TElementData >
DSS<TElementData>::DSS(IndexType NewId, GeometryType::Pointer pGeometry):
    FluidElement<TElementData>(NewId,pGeometry)
{}


template< class TElementData >
DSS<TElementData>::DSS(IndexType NewId, GeometryType::Pointer pGeometry, Properties::Pointer pProperties):
    FluidElement<TElementData>(NewId,pGeometry,pProperties)
{}


template< class TElementData >
DSS<TElementData>::~DSS()
{}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Public Operations

template< class TElementData >
Element::Pointer DSS<TElementData>::Create(IndexType NewId,NodesArrayType const& ThisNodes,Properties::Pointer pProperties) const
{
    return Element::Pointer(new DSS(NewId, this->GetGeometry().Create(ThisNodes), pProperties));
}


template< class TElementData >
Element::Pointer DSS<TElementData>::Create(IndexType NewId,GeometryType::Pointer pGeom,Properties::Pointer pProperties) const
{
    return Element::Pointer(new DSS(NewId, pGeom, pProperties));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Inquiry

template< class TElementData >
int DSS<TElementData>::Check(const ProcessInfo &rCurrentProcessInfo)
{
    int out = FluidElement<TElementData>::Check(rCurrentProcessInfo);

    // Extra variables

    // Output variables (for Calculate() functions)
    KRATOS_CHECK_VARIABLE_KEY(SUBSCALE_VELOCITY);
    KRATOS_CHECK_VARIABLE_KEY(SUBSCALE_PRESSURE);

    return out;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

template< class TElementData >
void DSS<TElementData>::GetValueOnIntegrationPoints(Variable<array_1d<double, 3 > > const& rVariable,
                                            std::vector<array_1d<double, 3 > >& rValues,
                                            ProcessInfo const& rCurrentProcessInfo)
{
    if (rVariable == SUBSCALE_VELOCITY)
    {
        // Get Shape function data
        Vector GaussWeights;
        Matrix ShapeFunctions;
        ShapeFunctionDerivativesArrayType ShapeDerivatives;
        this->CalculateGeometryData(GaussWeights,ShapeFunctions,ShapeDerivatives);
        const unsigned int NumGauss = GaussWeights.size();

        rValues.resize(NumGauss);

        TElementData data;
        data.Initialize(*this, rCurrentProcessInfo);

        for (unsigned int g = 0; g < NumGauss; g++)
        {
            IntegrationPointGeometryData integration_point(
                GaussWeights[g], row(ShapeFunctions, g), ShapeDerivatives[g]);

            this->SubscaleVelocity(data, integration_point, rCurrentProcessInfo,
                                   rValues[g]);
        }
    }
    else if (rVariable == VORTICITY)
    {
        // Get Shape function data
        Vector GaussWeights;
        Matrix ShapeFunctions;
        ShapeFunctionDerivativesArrayType ShapeDerivatives;
        this->CalculateGeometryData(GaussWeights,ShapeFunctions,ShapeDerivatives);
        const unsigned int NumGauss = GaussWeights.size();

        rValues.resize(NumGauss);

        for (unsigned int g = 0; g < NumGauss; g++)
        {
            this->IntegrationPointVorticity(ShapeDerivatives[g],rValues[g]);
        }
    }
}


template< class TElementData >
void DSS<TElementData>::GetValueOnIntegrationPoints(Variable<double> const& rVariable,
                                            std::vector<double>& rValues,
                                            ProcessInfo const& rCurrentProcessInfo)
{
    if (rVariable == SUBSCALE_PRESSURE)
    {
        // Get Shape function data
        Vector GaussWeights;
        Matrix ShapeFunctions;
        ShapeFunctionDerivativesArrayType ShapeDerivatives;
        this->CalculateGeometryData(GaussWeights,ShapeFunctions,ShapeDerivatives);
        const unsigned int NumGauss = GaussWeights.size();

        rValues.resize(NumGauss);

        TElementData data;
        data.Initialize(*this, rCurrentProcessInfo);

        for (unsigned int g = 0; g < NumGauss; g++)
        {
            IntegrationPointGeometryData integration_point(
                GaussWeights[g], row(ShapeFunctions, g), ShapeDerivatives[g]);

            this->SubscalePressure(data,integration_point,rCurrentProcessInfo,rValues[g]);
        }

    }
    else if (rVariable == Q_VALUE)
    {
		Vector GaussWeights;
		Matrix ShapeFunctions;
		ShapeFunctionDerivativesArrayType ShapeDerivatives;
		this->CalculateGeometryData(GaussWeights,ShapeFunctions,ShapeDerivatives);
		const unsigned int NumGauss = GaussWeights.size();

		rValues.resize(NumGauss);
		Matrix GradVel;

		// Loop on integration points
		for (unsigned int g = 0; g < NumGauss; g++)
		{
			GradVel = ZeroMatrix(Dim,Dim);
			const ShapeFunctionDerivativesType& rDN_DX = ShapeDerivatives[g];

			// Compute velocity gradient
			for (unsigned int i=0; i < Dim; ++i)
				for (unsigned int j=0; j < Dim; ++j)
					for (unsigned int iNode=0; iNode < NumNodes; ++iNode)
					{
						array_1d<double,3>& Vel =
							this->GetGeometry()[iNode].FastGetSolutionStepValue(VELOCITY);
						GradVel(i,j) += Vel[i] * rDN_DX(iNode,j);
					}

			// Compute Q-value
			double qval = 0.0;
			for (unsigned int i=0; i < Dim; ++i)
				for (unsigned int j=0; j < Dim; ++j)
					qval += GradVel(i,j) * GradVel(j,i);

			qval *= -0.5;
			rValues[g] = qval;
		}
	}
	else if (rVariable == VORTICITY_MAGNITUDE)
	{
		Vector GaussWeights;
		Matrix ShapeFunctions;
		ShapeFunctionDerivativesArrayType ShapeDerivatives;
		this->CalculateGeometryData(GaussWeights,ShapeFunctions,ShapeDerivatives);
		const unsigned int NumGauss = GaussWeights.size();

		rValues.resize(NumGauss);
		
  		// Loop on integration points
		for (unsigned int g = 0; g < NumGauss; g++)
		{
			const ShapeFunctionDerivativesType& rDN_DX = ShapeDerivatives[g];
			array_1d<double,3> Vorticity(3,0.0);

            this->IntegrationPointVorticity(rDN_DX,Vorticity);

			rValues[g] = sqrt(Vorticity[0] * Vorticity[0] + Vorticity[1] * Vorticity[1]
					+ Vorticity[2] * Vorticity[2]);
		}
	}
}

template <class TElementData>
void DSS<TElementData>::GetValueOnIntegrationPoints(Variable<array_1d<double, 6>> const& rVariable,
                                                    std::vector<array_1d<double, 6>>& rValues,
                                                    ProcessInfo const& rCurrentProcessInfo)
{
}

template <class TElementData>
void DSS<TElementData>::GetValueOnIntegrationPoints(Variable<Vector> const& rVariable,
                                                    std::vector<Vector>& rValues,
                                                    ProcessInfo const& rCurrentProcessInfo)
{
}

template <class TElementData>
void DSS<TElementData>::GetValueOnIntegrationPoints(Variable<Matrix> const& rVariable,
                                                    std::vector<Matrix>& rValues,
                                                    ProcessInfo const& rCurrentProcessInfo)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Input and output

template< class TElementData >
std::string DSS<TElementData>::Info() const
{
    std::stringstream buffer;
    buffer << "DSS #" << this->Id();
    return buffer.str();
}


template< class TElementData >
void DSS<TElementData>::PrintInfo(std::ostream& rOStream) const
{
    rOStream << "DSS" << Dim << "D";
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Protected functions

template< class TElementData >
void DSS<TElementData>::ASGSMomentumResidual(
    TElementData& rData,
    const IntegrationPointGeometryData& rIntegrationPoint,
    array_1d<double,3> &rMomentumRes)
{
    const GeometryType rGeom = this->GetGeometry();

    Vector AGradN;
    array_1d<double, 3> convective_velocity =
        this->Interpolate(rData.GetVELOCITY(), rIntegrationPoint.N) -
        this->Interpolate(rData.GetMESH_VELOCITY(), rIntegrationPoint.N);
    
    this->ConvectionOperator(AGradN,convective_velocity,rIntegrationPoint.DN_DX);

    double density = this->Interpolate(rData.GetDENSITY(),rIntegrationPoint.N);
    const auto& r_body_forces = rData.GetBODY_FORCE().Data();
    const auto& r_velocities = rData.GetVELOCITY().Data();
    const auto& r_pressures = rData.GetPRESSURE().Data();

    for (unsigned int i = 0; i < NumNodes; i++)
    {
        const array_1d<double,3>& rAcc = rGeom[i].FastGetSolutionStepValue(ACCELERATION);

        for (unsigned int d = 0; d < Dim; d++)
        {
            rMomentumRes[d] += density * ( rIntegrationPoint.N[i]*(r_body_forces(i,d) - rAcc[d]) - AGradN[i]*r_velocities(i,d)) - rIntegrationPoint.DN_DX(i,d)*r_pressures[i];
        }
    }
}


template< class TElementData >
void DSS<TElementData>::ASGSMassResidual(
    TElementData& rData,
    const IntegrationPointGeometryData& rIntegrationPoint,
    double &rMomentumRes)
{
    this->MassProjTerm(rData,rIntegrationPoint,rMomentumRes);
}


template< class TElementData >
void DSS<TElementData>::OSSMomentumResidual(
    TElementData& rData,
    const IntegrationPointGeometryData& rIntegrationPoint,
    array_1d<double,3> &rMomentumRes)
{
    this->MomentumProjTerm(rData,rIntegrationPoint,rMomentumRes);

    array_1d<double,3> momentum_projection = this->Interpolate(rData.GetADVPROJ(),rIntegrationPoint.N);
    for (unsigned int d = 0; d < Dim; d++)
        rMomentumRes[d] -= momentum_projection[d];
}


template< class TElementData >
void DSS<TElementData>::OSSMassResidual(
    TElementData& rData,
    const IntegrationPointGeometryData& rIntegrationPoint,
    double &rMassRes)
{
    this->MassProjTerm(rData,rIntegrationPoint,rMassRes);
    double mass_projection = this->Interpolate(rData.GetDIVPROJ(),rIntegrationPoint.N);
    rMassRes -= mass_projection;
}


template< class TElementData >
void DSS<TElementData>::MomentumProjTerm(
    TElementData& rData,
    const IntegrationPointGeometryData& rIntegrationPoint,
    array_1d<double,3> &rMomentumRHS)
{
        array_1d<double, 3> convective_velocity =
        this->Interpolate(rData.GetVELOCITY(), rIntegrationPoint.N) -
        this->Interpolate(rData.GetMESH_VELOCITY(), rIntegrationPoint.N);
    
    Vector AGradN;
    this->ConvectionOperator(AGradN,convective_velocity,rIntegrationPoint.DN_DX);

    double density = this->Interpolate(rData.GetDENSITY(),rIntegrationPoint.N);

    for (unsigned int i = 0; i < NumNodes; i++)
    {
        for (unsigned int d = 0; d < Dim; d++)
        {
            rMomentumRHS[d] += density * ( rIntegrationPoint.N[i]*(rData.GetBODY_FORCE().Data()(i,d) /*- rAcc[d]*/) - AGradN[i]*rData.GetVELOCITY().Data()(i,d)) - rIntegrationPoint.DN_DX(i,d)*rData.GetPRESSURE().Data()[i];
        }
    }
}


template< class TElementData >
void DSS<TElementData>::MassProjTerm(
    TElementData& rData,
    const IntegrationPointGeometryData& rIntegrationPoint,
    double &rMassRHS)
{
    for (unsigned int i = 0; i < NumNodes; i++)
    {
        for (unsigned int d = 0; d < Dim; d++)
            rMassRHS -= rIntegrationPoint.DN_DX(i,d)*rData.GetVELOCITY().Data()(i,d);
    }

}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Evaluation of system terms on Gauss Points

template< class TElementData >
void DSS<TElementData>::AddSystemTerms(
    TElementData& rData,
    const IntegrationPointGeometryData& rIntegrationPoint,
    const ProcessInfo &rProcessInfo,
    MatrixType &rLHS,
    VectorType &rRHS)
{
    // Interpolate nodal data on the integration point
    double ElemSize = this->ElementSize();

    double density = this->Interpolate(rData.GetDENSITY(),rIntegrationPoint.N);
    double viscosity = this->EffectiveViscosity(rData,rIntegrationPoint,ElemSize,rProcessInfo);
    array_1d<double,3> body_force = this->Interpolate(rData.GetBODY_FORCE(),rIntegrationPoint.N);
    array_1d<double,3> momentum_projection = this->Interpolate(rData.GetADVPROJ(),rIntegrationPoint.N);
    double mass_projection = this->Interpolate(rData.GetDIVPROJ(),rIntegrationPoint.N);

    double TauOne;
    double TauTwo;
    array_1d<double, 3> convective_velocity =
        this->Interpolate(rData.GetVELOCITY(), rIntegrationPoint.N) -
        this->Interpolate(rData.GetMESH_VELOCITY(), rIntegrationPoint.N);
        
    this->CalculateStaticTau(density,viscosity,convective_velocity,ElemSize,rProcessInfo,TauOne,TauTwo);

    Vector AGradN;
    this->ConvectionOperator(AGradN,convective_velocity,rIntegrationPoint.DN_DX);
    
    // Multiplying some quantities by density to have correct units
    viscosity *= density; // Dynamic viscosity
    body_force *= density; // Force per unit of volume
    AGradN *= density; // Convective term is always multiplied by density

    // Auxiliary variables for matrix looping
    unsigned int Row = 0;
    unsigned int Col = 0;

    // Temporary containers
    double K,L,G,PDivV,qF;

    // Note: Dof order is (u,v,[w,]p) for each node
    for (unsigned int i = 0; i < NumNodes; i++)
    {
        Row = i*BlockSize;

        // LHS terms
        for (unsigned int j = 0; j < NumNodes; j++)
        {
            Col = j*BlockSize;

            // Some terms are the same for all velocity components, calculate them once for each i,j
            //K = 0.5*(rN[i]*AGradN[j] - AGradN[i]*rN[j]); // Skew-symmetric convective term 1/2( v*grad(u)*u - grad(v) uu )
            K = rIntegrationPoint.N[i]*AGradN[j];
            K += AGradN[i]*TauOne*(AGradN[j]); // Stabilization: u*grad(v) * TauOne * u*grad(u)
            K *= rIntegrationPoint.Weight;

            // q-p stabilization block (reset result)
            L = 0;

            // The following lines implement the viscous term as a Laplacian
            //for (unsigned int d = 0; d < Dim; d++)
            //    K += GaussWeight * Density * Viscosity * rDN_DX(i, d) * rDN_DX(j, d);

            for (unsigned int d = 0; d < Dim; d++)
            {
                //K += GaussWeight * Density * Viscosity * rDN_DX(i, d) * rDN_DX(j, d);
                rLHS(Row+d,Col+d) += K;

                // v * Grad(p) block
                G = TauOne * AGradN[i] * rIntegrationPoint.DN_DX(j,d); // Stabilization: (a * Grad(v)) * TauOne * Grad(p)
                PDivV = rIntegrationPoint.DN_DX(i,d) * rIntegrationPoint.N[j]; // Div(v) * p

                // Write v * Grad(p) component
                rLHS(Row+d,Col+Dim) += rIntegrationPoint.Weight * (G - PDivV);
                // Use symmetry to write the q * Div(u) component
                rLHS(Col+Dim,Row+d) += rIntegrationPoint.Weight * (G + PDivV);

                // q-p stabilization block
                L += rIntegrationPoint.DN_DX(i,d) * rIntegrationPoint.DN_DX(j,d); // Stabilization: Grad(q) * TauOne * Grad(p)

                for (unsigned int e = 0; e < Dim; e++) // Stabilization: Div(v) * TauTwo * Div(u)
                    rLHS(Row+d,Col+e) += rIntegrationPoint.Weight*TauTwo*rIntegrationPoint.DN_DX(i,d)*rIntegrationPoint.DN_DX(j,e);
            }

            // Write q-p term
            rLHS(Row+Dim,Col+Dim) += rIntegrationPoint.Weight*TauOne*L;
        }

        // RHS terms
        qF = 0.0;
        for (unsigned int d = 0; d < Dim; ++d)
        {
            rRHS[Row+d] += rIntegrationPoint.Weight * rIntegrationPoint.N[i] * body_force[d]; // v*BodyForce
            rRHS[Row+d] += rIntegrationPoint.Weight * TauOne * AGradN[i] * ( body_force[d] - momentum_projection[d]); // ( a * Grad(v) ) * TauOne * (Density * BodyForce)
            rRHS[Row+d] -= rIntegrationPoint.Weight * TauTwo * rIntegrationPoint.DN_DX(i,d) * mass_projection;
            qF += rIntegrationPoint.DN_DX(i, d) * (body_force[d] - momentum_projection[d]);
        }
        rRHS[Row + Dim] += rIntegrationPoint.Weight * TauOne * qF; // Grad(q) * TauOne * (Density * BodyForce)
    }

    // Viscous contribution (with symmetric gradient 2*nu*{E(u) - 1/3 Tr(E)} )
    // This could potentially be optimized, as it can be integrated exactly using one less integration order when compared to previous terms.
    Internals::AddViscousTerm<Dim>(viscosity,rIntegrationPoint.Weight,rIntegrationPoint.DN_DX,rLHS);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

template< class TElementData >
void DSS<TElementData>::AddMassTerms(
    TElementData& rData,
    const IntegrationPointGeometryData& rIntegrationPoint,
    const ProcessInfo &rProcessInfo,
    MatrixType &rMassMatrix)
{
    unsigned int Row = 0;
    unsigned int Col = 0;

    double density = this->Interpolate(rData.GetDENSITY(),rIntegrationPoint.N);

    // Note: Dof order is (u,v,[w,]p) for each node
    for (unsigned int i = 0; i < NumNodes; i++)
    {
        Row = i*BlockSize;
        for (unsigned int j = 0; j < NumNodes; j++)
        {
            Col = j*BlockSize;
            const double Mij = rIntegrationPoint.Weight * density * rIntegrationPoint.N[i] * rIntegrationPoint.N[j];
            for (unsigned int d = 0; d < Dim; d++)
                rMassMatrix(Row+d,Col+d) += Mij;
        }
    }

    /* Note on OSS and full projection: Riccardo says that adding the terms provided by
     * AddMassStabilization (and incluiding their corresponding terms in the projeciton)
     * could help reduce the non-linearity of the coupling between projection and u,p
     * However, leaving them on gives a lot of trouble whith the Bossak scheme:
     * think that we solve F - (1-alpha)*M*u^(n+1) - alpha*M*u^(n) - K(u^(n+1)) = 0
     * so the projection of the dynamic terms should be Pi( (1-alpha)*u^(n+1) - alpha*u^(n) )
     */
    if ( rProcessInfo[OSS_SWITCH] != 1.0 )
        this->AddMassStabilization(rData,rIntegrationPoint,rProcessInfo,rMassMatrix);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

template< class TElementData >
void DSS<TElementData>::AddMassStabilization(
    TElementData& rData,
    const IntegrationPointGeometryData& rIntegrationPoint,
    const ProcessInfo &rProcessInfo,
    MatrixType &rMassMatrix)
{
    double ElemSize = this->ElementSize();

    double density = this->Interpolate(rData.GetDENSITY(),rIntegrationPoint.N);
    double viscosity = this->EffectiveViscosity(rData,rIntegrationPoint,ElemSize,rProcessInfo);

    double TauOne;
    double TauTwo;
    array_1d<double,3> convective_velocity = this->Interpolate(rData.GetVELOCITY(),rIntegrationPoint.N) - this->Interpolate(rData.GetMESH_VELOCITY(),rIntegrationPoint.N);
    this->CalculateStaticTau(density,viscosity,convective_velocity,ElemSize,rProcessInfo,TauOne,TauTwo);

    Vector AGradN;
    this->ConvectionOperator(AGradN,convective_velocity,rIntegrationPoint.DN_DX);

    // Multiplying some quantities by density to have correct units
    AGradN *= density; // Convective term is always multiplied by density

    // Auxiliary variables for matrix looping
    unsigned int Row = 0;
    unsigned int Col = 0;

    // Temporary container
    double K;
    double W = rIntegrationPoint.Weight * TauOne * density; // This density is for the dynamic term in the residual (rho*Du/Dt)

    // Note: Dof order is (u,v,[w,]p) for each node
    for (unsigned int i = 0; i < NumNodes; i++)
    {
        Row = i*BlockSize;

        for (unsigned int j = 0; j < NumNodes; j++)
        {
            Col = j*BlockSize;

            K = W * AGradN[i] * rIntegrationPoint.N[j];

            for (unsigned int d = 0; d < Dim; d++)
            {
                rMassMatrix(Row+d,Col+d) += K;
                rMassMatrix(Row+Dim,Col+d) += W*rIntegrationPoint.DN_DX(i,d)*rIntegrationPoint.N[j];
            }
        }
    }
}

template< class TElementData >
void DSS<TElementData>::CalculateProjections(const ProcessInfo &rCurrentProcessInfo)
{
    // Get Shape function data
    Vector GaussWeights;
    Matrix ShapeFunctions;
    ShapeFunctionDerivativesArrayType ShapeDerivatives;
    this->CalculateGeometryData(GaussWeights,ShapeFunctions,ShapeDerivatives);
    const unsigned int NumGauss = GaussWeights.size();

    GeometryType& rGeom = this->GetGeometry();
    VectorType MomentumRHS = ZeroVector(NumNodes * Dim);
    VectorType MassRHS = ZeroVector(NumNodes);
    VectorType NodalArea = ZeroVector(NumNodes);

    TElementData data;
    data.Initialize(*this, rCurrentProcessInfo);

    for (unsigned int g = 0; g < NumGauss; g++)
    {
        IntegrationPointGeometryData integration_point(
            GaussWeights[g], row(ShapeFunctions, g), ShapeDerivatives[g]);

        array_1d<double, 3> MomentumRes(3, 0.0);
        double MassRes = 0.0;

        this->MomentumProjTerm(data, integration_point, MomentumRes);
        this->MassProjTerm(data,integration_point,MassRes);

        for (unsigned int i = 0; i < NumNodes; i++)
        {
            double W = integration_point.Weight*integration_point.N[i];
            unsigned int Row = i*Dim;
            for (unsigned int d = 0; d < Dim; d++)
                MomentumRHS[Row+d] += W*MomentumRes[d];
            NodalArea[i] += W;
            MassRHS[i] += W*MassRes;
        }
    }

    // Add carefully to nodal variables to avoid OpenMP race condition
    unsigned int Row = 0;
    for (SizeType i = 0; i < NumNodes; ++i)
    {
        rGeom[i].SetLock(); // So it is safe to write in the node in OpenMP
        array_1d<double,3>& rMomValue = rGeom[i].FastGetSolutionStepValue(ADVPROJ);
        for (unsigned int d = 0; d < Dim; ++d)
            rMomValue[d] += MomentumRHS[Row++];
        rGeom[i].FastGetSolutionStepValue(DIVPROJ) += MassRHS[i];
        rGeom[i].FastGetSolutionStepValue(NODAL_AREA) += NodalArea[i];
        rGeom[i].UnSetLock(); // Free the node for other threads
    }
}

template< class TElementData >
void DSS<TElementData>::SubscaleVelocity(
    TElementData& rData,
    const IntegrationPointGeometryData& rIntegrationPoint,
    const ProcessInfo &rProcessInfo,
    array_1d<double,3> &rVelocitySubscale)
{
    double ElemSize = this->ElementSize();
    double Viscosity = this->EffectiveViscosity(rData,rIntegrationPoint,ElemSize,rProcessInfo);

    double TauOne;
    double TauTwo;
    double density = this->Interpolate(rData.GetDENSITY(),rIntegrationPoint.N);
    array_1d<double,3> convective_velocity = this->Interpolate(rData.GetVELOCITY(),rIntegrationPoint.N) - this->Interpolate(rData.GetMESH_VELOCITY(),rIntegrationPoint.N);
    this->CalculateStaticTau(density,Viscosity,convective_velocity,ElemSize,rProcessInfo,TauOne,TauTwo);

    array_1d<double,3> Residual(3,0.0);

    if (rProcessInfo[OSS_SWITCH] != 1.0)
        this->ASGSMomentumResidual(rData,rIntegrationPoint,Residual);
    else
        this->OSSMomentumResidual(rData,rIntegrationPoint,Residual);

    rVelocitySubscale = TauOne*Residual;
}

template< class TElementData >
void DSS<TElementData>::SubscalePressure(
        TElementData& rData,
        const IntegrationPointGeometryData& rIntegrationPoint,
        const ProcessInfo& rProcessInfo,
        double &rPressureSubscale)
{
    //double ElemSize = this->ElementSize(ConvVel,rDN_DX);
    double ElemSize = this->ElementSize();
    double Viscosity = this->EffectiveViscosity(rData,rIntegrationPoint,ElemSize,rProcessInfo);

    double TauOne;
    double TauTwo;
    double density = this->Interpolate(rData.GetDENSITY(),rIntegrationPoint.N);
    array_1d<double, 3> convective_velocity =
        this->Interpolate(rData.GetVELOCITY(), rIntegrationPoint.N) -
        this->Interpolate(rData.GetMESH_VELOCITY(), rIntegrationPoint.N);
    this->CalculateStaticTau(density, Viscosity, convective_velocity, ElemSize,
                             rProcessInfo, TauOne, TauTwo);

    double Residual = 0.0;

    if (rProcessInfo[OSS_SWITCH] != 1.0)
        this->ASGSMassResidual(rData,rIntegrationPoint,Residual);
    else
        this->OSSMassResidual(rData,rIntegrationPoint,Residual);

    rPressureSubscale = TauTwo*Residual;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Private functions
///////////////////////////////////////////////////////////////////////////////////////////////////

// serializer

template< class TElementData >
void DSS<TElementData>::save(Serializer& rSerializer) const
{
    typedef FluidElement<TElementData> BaseElement;
    KRATOS_SERIALIZE_SAVE_BASE_CLASS(rSerializer, BaseElement );
}


template< class TElementData >
void DSS<TElementData>::load(Serializer& rSerializer)
{
    typedef FluidElement<TElementData> BaseElement;
    KRATOS_SERIALIZE_LOAD_BASE_CLASS(rSerializer, BaseElement);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Internals
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace Internals {

///////////////////////////////////////////////////////////////////////////////////////////////////
// For Dim == 2
///////////////////////////////////////////////////////////////////////////////////////////////////

template <>
void AddViscousTerm<2>(double DynamicViscosity,
                       double GaussWeight,
                       const Kratos::Matrix& rDN_DX,
                       Kratos::Matrix& rLHS)
{
    double weight = GaussWeight * DynamicViscosity;

    constexpr double four_thirds = 4.0 / 3.0;
    constexpr double minus_two_thirds = -2.0 / 3.0;

    const unsigned int num_nodes = rDN_DX.size1();
    const unsigned int block_size = 3;

    unsigned int row(0),col(0);

    for (unsigned int a = 0; a < num_nodes; ++a)
    {
        row = a*block_size;
        for (unsigned int b = 0; b < num_nodes; ++b)
        {
            col = b*block_size;

            // First row
            rLHS(row,col) += weight * ( four_thirds * rDN_DX(a,0) * rDN_DX(b,0) + rDN_DX(a,1) * rDN_DX(b,1) );
            rLHS(row,col+1) += weight * ( minus_two_thirds * rDN_DX(a,0) * rDN_DX(b,1) + rDN_DX(a,1) * rDN_DX(b,0) );

            // Second row
            rLHS(row+1,col) += weight * ( minus_two_thirds * rDN_DX(a,1) * rDN_DX(b,0) + rDN_DX(a,0) * rDN_DX(b,1) );
            rLHS(row+1,col+1) += weight * ( four_thirds * rDN_DX(a,1) * rDN_DX(b,1) + rDN_DX(a,0) * rDN_DX(b,0) );
        }
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// For Dim == 3
///////////////////////////////////////////////////////////////////////////////////////////////////

template <>
void AddViscousTerm<3>(double DynamicViscosity,
                       double GaussWeight,
                       const Kratos::Matrix& rDN_DX,
                       Kratos::Matrix& rLHS)
{
    double weight = GaussWeight * DynamicViscosity;

    constexpr double one_third = 1.0 / 3.0;
    constexpr double minus_two_thirds = -2.0 / 3.0;

    const unsigned int num_nodes = rDN_DX.size1();
    const unsigned int block_size = 4;

    unsigned int row(0),col(0);

    for (unsigned int i = 0; i < num_nodes; ++i)
    {
        row = i*block_size;
        for (unsigned int j = 0; j < num_nodes; ++j)
        {
            col = j*block_size;
            // (dN_i/dx_k dN_j/dx_k)
            const double diag =  rDN_DX(i,0) * rDN_DX(j,0) + rDN_DX(i,1) * rDN_DX(j,1) + rDN_DX(i,2) * rDN_DX(j,2);

            // First row
            rLHS(row,col) += weight * ( one_third * rDN_DX(i,0) * rDN_DX(j,0) + diag );
            rLHS(row,col+1) += weight * ( minus_two_thirds * rDN_DX(i,0) * rDN_DX(j,1) + rDN_DX(i,1) * rDN_DX(j,0) );
            rLHS(row,col+2) += weight * ( minus_two_thirds * rDN_DX(i,0) * rDN_DX(j,2) + rDN_DX(i,2) * rDN_DX(j,0) );

            // Second row
            rLHS(row+1,col) += weight * ( minus_two_thirds * rDN_DX(i,1) * rDN_DX(j,0) + rDN_DX(i,0) * rDN_DX(j,1) );
            rLHS(row+1,col+1) += weight * ( one_third * rDN_DX(i,1) * rDN_DX(j,1) + diag );
            rLHS(row+1,col+2) += weight * ( minus_two_thirds * rDN_DX(i,1) * rDN_DX(j,2) + rDN_DX(i,2) * rDN_DX(j,1) );

            // Third row
            rLHS(row+2,col) += weight * ( minus_two_thirds * rDN_DX(i,2) * rDN_DX(j,0) + rDN_DX(i,0) * rDN_DX(j,2) );
            rLHS(row+2,col+1) += weight * ( minus_two_thirds * rDN_DX(i,2) * rDN_DX(j,1) + rDN_DX(i,1) * rDN_DX(j,2) );
            rLHS(row+2,col+2) += weight * ( one_third * rDN_DX(i,2) * rDN_DX(j,2) + diag );
        }
    }
}
}

}