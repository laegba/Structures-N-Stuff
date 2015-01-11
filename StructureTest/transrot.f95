module transrot
!
!    module transrot handles translation and rotation of coordinate systems
!    Copyright (C) 2015  Michael Salay,   Mike.Salay at gmail
!
!    This program is free software: you can redistribute it and/or modify
!    it under the terms of the GNU General Public License as published by
!    the Free Software Foundation, either version 3 of the License, or
!    (at your option) any later version.
!
!    This program is distributed in the hope that it will be useful,
!    but WITHOUT ANY WARRANTY; without even the implied warranty of
!    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
!    GNU General Public License for more details.
!
!    You should have received a copy of the GNU General Public License
!    along with this program.  If not, see <http://www.gnu.org/licenses/>.
!
!
! Fortran uses Column-Major Matrix Ordering
! C++ uses Row-Major Matrix Ordering
!
!   This module was intended to be used with c++ so Row-Major ordering was used
!   by reversing index order to use c++ Arrays ordering:
!
!      i.e. (column j, row i)
!
! However:
!   OGL uses Right Handed Coordinate System and Column-Major Matrices
!   So may eventually want to shift to Column-Major to write directly to OpenGL
!     and switch the indices in C++.
!
!            
!  Usage:
!          notes: 
!
!                1) reversed index order (j column,i row)
!
!                2) rotations used for a right handed coordinate system are left handed
!                so that the transformation converts a direction or point in global
!                coordinates to local coordinates
!
!                Should rename routines to be clearer about what is being done
!
!                Should ensure that it is clear how to convert both ways
!
!        
!    For full transformation need to define size 3 position and rotation vectors:
!        1) a T(4,3) transformation array
!        2) an R(3,3) rotation array
!        3) an position vector x(3)
!
!    A) A full transformation matrix can be calculated and written to T
!       from a vector representing position and rotation
!         uv(6)= [position]x,y,z, [rotation]u,v,w
!         by:
!                  call transrotangsetf(T,uv)
!
!    B) An existing transformation matrix T can be varied
!
!         Reset x to 0 and T to the identity matrix by:
!                  call trinitf(x,T)
!
!         Can then successively translate and rotate existing matrix:
!
!           Rotate T in any local direction r(3) by:
!                  call rotatef(T,r,x)
!
!           Translate T in any local direction u(3) by:
!                  x=translatef(T,u,x)  
!
!           Rotate then translate by:
!                 x=transrotangf(T,u,r,x)  
!
!           Add trans then rot function?
!           Add set transformation matrix from 2 points preserving up?
!
!
!    B) The transformtion T can be applied to any direction or position.
!
!         To transform a direction r(3) to a local direction rl(3):
!                rl=rotvf(T,r)
!
!         To transform a position p(3) to a local direction pl(3):
!                pl=transvf(T,p)
!

implicit none

 real,parameter :: worldup(3)=(/ 0.,1.,0. /)

contains


subroutine trinitf(x,T) !inititalize variables
  real, dimension(4,3) :: T
  real, dimension(3)   :: x
   x=0.; T=0.; T(1:3,:)=identf(3) !inititalize variables
end subroutine trinitf


function identf(n)
  real, dimension(n,n) :: identf
  integer :: i,n
  identf=0.
  forall(i = 1:n) identf(i,i) = 1.
end function identf


function normv(u)
  real, dimension(3) :: normv,u !normalized vector
  real :: a
  a=sqrt(u(1)**2.+u(2)**2.+u(3)**2.)
  normv=u/a
end function normv


function crossf(u, v) ! 3D vector cross product u x v (handedness independent)
  real, dimension(3) :: crossf, u, v
  crossf(1) = u(2) * v(3) - u(3) * v(2)
  crossf(2) = u(3) * v(1) - u(1) * v(3)
  crossf(3) = u(1) * v(2) - u(2) * v(1)
end function crossf


function crossrf(u, r) ! 3D vector cross product u and rotation)
  real, dimension(3) :: crossrf, u,r
  crossrf(1) =  u(3)*sin(r(2)) + u(1)*cos(r(2))&
               +u(1)*cos(r(3)) - u(2)*sin(r(3))
  crossrf(2) =  u(2)*cos(r(1)) - u(3)*sin(r(1))&
               +u(1)*sin(r(3)) + u(2)*cos(r(3))
  crossrf(3) =  u(2)*sin(r(1)) + u(3)*cos(r(1))&
               +u(3)*cos(r(2)) - u(1)*sin(r(2))
end function crossrf


function dotvf(a,v) ! Calculate dot product of matrix and vector
  real, dimension(3,3) :: a
  real, dimension(3)   :: v, dotvf
  integer              :: i
  forall(i = 1:3) dotvf(i)=dot_product(a(:,i),v)
end function dotvf


function dotf(a,b) ! Calculate dot product two 3D matrices
  real, dimension(3,3)   :: a,b, dotf
  integer                :: i,j
  forall(i = 1:3,j=1:3) dotf(j,i)=dot_product(a(:,i),b(j,:))
end function dotf


function rotmp(p0,p1) ! Calculate rotation matrix from 2 points
  real :: rotmp(3,3)
  real, dimension(3) :: p0,p1
  rotmp=0.
  rotmp(:,3)=normv(p1-p0)                                         ! new z-direction
  rotmp(:,2)=worldup - dot_product(worldup,rotmp(:,3))*rotmp(:,3) ! new y-direction
  rotmp(:,1)=crossf(rotmp(:,2),rotmp(:,3))                        ! new x-direction
end function rotmp


function rotangrf(u) ! Calculate rotation matrix about direction u
  real, dimension(3,3) :: rotangrf 
  real, dimension(3) :: u
  real :: a
  a=sqrt(u(1)**2.+u(2)**2.+u(3)**2.)
  rotangrf=rotangdrf(u/(a+2.e-32),a)
  
  
end function rotangrf


function rotangdrf(u,a) ! Calculate rotation matrix for angle a about direction u
  real, dimension(3,3) :: rotangdrf 
  real, dimension(3) :: u
  real :: a,c,s,t
  c=cos(a)
  s=-sin(a)
  t=1.-cos(a)
  rotangdrf(1,1) = t*u(1)**2. + c
  rotangdrf(2,1) = t*u(1)*u(2) - s*u(3)
  rotangdrf(3,1) = t*u(1)*u(3) + s*u(2)
  
  rotangdrf(1,2) = t*u(1)*u(2) + s*u(3)
  rotangdrf(2,2) = t*u(2)**2. + c
  rotangdrf(3,2) = t*u(2)*u(3) - s*u(1)

  rotangdrf(1,3) = t*u(1)*u(3) - s*u(2)
  rotangdrf(2,3) = t*u(2)*u(3) + s*u(1)
  rotangdrf(3,3) = t*u(3)**2. + c
end function rotangdrf


function rotxrf(r) ! Calculate rotation matrix based on angle about x
  real, dimension(3,3) :: rotxrf 
  real                 :: r
  rotxrf=0.
  rotxrf(1,1) = 1.
  rotxrf(2,2) = +cos(r) 
  rotxrf(3,2) = +sin(r)
  rotxrf(2,3) = -sin(r)
  rotxrf(3,3) = +cos(r)
end function rotxrf


function rotyrf(r) ! Calculate rotation matrix based on angle r
  real, dimension(3,3) :: rotyrf 
  real                 :: r
  rotyrf=0.
  rotyrf(2,2) = 1.
  rotyrf(1,1) = +cos(r)
  rotyrf(3,1) = -sin(r) 
  rotyrf(1,3) = +sin(r)
  rotyrf(3,3) = +cos(r) 
end function rotyrf


function rotzrf(r) ! Calculate rotation matrix based on angle r
  real, dimension(3,3) :: rotzrf 
  real                 :: r
  rotzrf=0.
  rotzrf(3,3) = 1.
  rotzrf(1,1) = +cos(r) 
  rotzrf(2,1) = +sin(r)
  rotzrf(1,2) = -sin(r) 
  rotzrf(2,2) = +cos(r) 
end function rotzrf


subroutine applyrotf(T,R)
  real, dimension(4,3) :: T
  real, dimension(3,3) :: R
  T(1:3,1:3)=dotf(R,T(1:3,1:3)) ! apply rotation to matrix
end subroutine applyrotf


subroutine transrotf(T,R,v)     ! rotate and translate
  real, dimension(4,3) :: T
  real, dimension(3,3) :: R
  real, dimension(3)   :: v
  T(1:3,1:3)=dotf(R,T(1:3,1:3)) ! apply rotation matrix
  T(4,:)=-rotvf(T,v)             ! translate updated rotation by
end subroutine transrotf


subroutine transf(T,v) ! 3D transform matrix translate T by v
  real, dimension(4,3) :: T
  real, dimension(3)   :: v
  integer              :: i
  forall(i = 1:3) T(4,i)=dot_product(T(1:3,i),v)+T(4,i)
end subroutine transf


function rotvf(T,v) ! 3D vector transform T by v by calculating dot product
  real, dimension(4,3) :: T
  real, dimension(3)   :: v,rotvf
  integer              :: i
  forall(i = 1:3) rotvf(i)=dot_product(T(1:3,i),v)
end function rotvf

function rotvtf(T,v) ! 3D vector transform T by v by calculating dot product
  real, dimension(4,3) :: T
  real, dimension(3)   :: v,rotvtf
  integer              :: j
  forall(j = 1:3) rotvtf(j)=dot_product(T(j,1:3),v) ! inverted index order
end function rotvtf


function transvf(T,v) ! 3D vector transform T by v by calculating dot product
  real, dimension(4,3) :: T
  real, dimension(3)   :: v,transvf
  transvf=rotvf(T,v)+T(4,1:3) ! add last row
end function transvf


subroutine rotatef(T,w,x)      ! translate, returns position
  real, dimension(4,3) :: T
  real, dimension(3,3) :: R
  real, dimension(3)   :: w,x
  R=rotangrf(w)                 ! Rotation matrix based on angle
  T(1:3,1:3)=dotf(R,T(1:3,1:3)) ! apply rotation matrix
  T(4,:)=-rotvf(T,x)             ! translate updated rotation
end subroutine rotatef


function translatef(T,u,x)      ! translate, returns position
  real, dimension(4,3) :: T
  real, dimension(3)   :: u,x,translatef
  integer j
  translatef=x+rotvtf(T,u)      ! update position x
  T(4,:)=-rotvf(T,translatef)   ! apply translation to matrix
end function translatef


function transrotangf(T,u,w,x)   ! rotate and translate, returns position
  real, dimension(4,3) :: T
  real, dimension(3,3) :: R
  real, dimension(3)   :: u,w,x,transrotangf
  R=rotangrf(w)  
  T(1:3,1:3)=dotf(R,T(1:3,1:3)) ! apply rotation matrix
  transrotangf=x+rotvtf(T,u)     ! update position x
  T(4,:)=-rotvf(T,transrotangf)   
end function transrotangf


subroutine transrotangsetf(T,uv)   ! create new rotation matrix from combined
  real, dimension(4,3) :: T        ! location and rotation vector
  real, dimension(6)   :: uv
  T(1:3,1:3)=rotangrf(uv(4:6))      ! set rotation matrix
  T(4,:)=-rotvf(T,uv(1:3))          ! set translation matrix
end subroutine transrotangsetf


! XXX XXX XXX XXX XXX XXX XXX XXX XXX
! XXX      Replaced Routines      XXX
! XXX XXX XXX XXX XXX XXX XXX XXX XXX
!
! May eventually want to get rid of these.  

! XXX LEFT HANDED ROUTINES
               
function crosslf(u, r) ! 3D vector cross product u and rotation (left handed)
  real, dimension(3) :: crosslf, u, r
  crosslf(1) = -u(3)*sin(r(2)) + u(1)*cos(r(2))&
               +u(1)*cos(r(3)) + u(2)*sin(r(3))
  crosslf(2) =  u(2)*cos(r(1)) + u(3)*sin(r(1))&
               -u(1)*sin(r(3)) + u(2)*cos(r(3))
  crosslf(3) = -u(2)*sin(r(1)) + u(3)*cos(r(1))&
               +u(3)*cos(r(2)) + u(1)*sin(r(2))
end function crosslf

function rotxlf(s) ! Calculate rotation matrix based on angle r (left handed)
! note reversed index order (j column,i row)
  real, dimension(3,3) :: rotxlf 
  real                 :: s
  rotxlf=0.
  rotxlf(1,1) = 1.
  rotxlf(2,2) = +cos(s) 
  rotxlf(3,2) = -sin(s)
  rotxlf(2,3) = +sin(s)
  rotxlf(3,3) = +cos(s)
end function rotxlf

function rotylf(r) ! Calculate rotation matrix based on angle r
! note reversed index order (j column,i row)
  real, dimension(3,3) :: rotylf 
  real                 :: r
  rotylf=0.
  rotylf(2,2) = 1.
  rotylf(1,1) = +cos(r)
  rotylf(3,1) = +sin(r) 
  rotylf(1,3) = -sin(r)
  rotylf(3,3) = +cos(r) 
end function rotylf

function rotzlf(r) ! Calculate rotation matrix based on angle r
! note reversed index order (j column,i row)
  real, dimension(3,3) :: rotzlf 
  real                 :: r
  rotzlf=0.
  rotzlf(3,3) = 1.
  rotzlf(1,1) = +cos(r) 
  rotzlf(2,1) = -sin(r)
  rotzlf(1,2) = +sin(r) 
  rotzlf(2,2) = +cos(r) 
end function rotzlf


function rotanglf(u) ! Calculate rotation matrix about direction u, left handed
  real, dimension(3,3) :: rotanglf 
  real, dimension(3) :: u
  real :: a
  a=sqrt(u(1)**2.+u(2)**2.+u(3)**2.)
  rotanglf=rotangdlf(u/(a+2.e-32),a)
end function rotanglf


function rotangdlf(u,a) ! Calculate rotation matrix for angle a about direction u
! XXX left handed
  real, dimension(3,3) :: rotangdlf 
  real, dimension(3) :: u
  real :: a,c,s,t
  c=cos(a)
  s=sin(a)
  t=1.-cos(a)
  rotangdlf(1,1) = t*u(1)**2. + c
  rotangdlf(2,1) = t*u(1)*u(2) - s*u(3)
  rotangdlf(3,1) = t*u(1)*u(3) + s*u(2)
  
  rotangdlf(1,2) = t*u(1)*u(2) + s*u(3)
  rotangdlf(2,2) = t*u(2)**2. + c
  rotangdlf(3,2) = t*u(2)*u(3) - s*u(1)

  rotangdlf(1,3) = t*u(1)*u(3) - s*u(2)
  rotangdlf(2,3) = t*u(2)*u(3) + s*u(1)
  rotangdlf(3,3) = t*u(3)**2. + c
end function rotangdlf


! XXX USES VECTOR SIZE 4

subroutine translf(T,v) ! 3D vector trans44form T by v by calculating dot product
! XXX note need extra size for v when translated away from 0,0,0 XXX
!
! XXX replaced by trans44f
  real, dimension(4,4) :: T
  real, dimension(3)   :: v
  integer              :: i
  forall(i = 1:3) T(4,i)=dot_product(T(1:3,i),v)
end subroutine translf


function transv4f(T,v) ! 3D vector trans44form T by v by calculating dot product
! XXX REPLACED BY TRANSVF  - only preserved for example XXX
  real, dimension(4,4) :: T
  real, dimension(4)   :: v,transv4f
  integer              :: i
  forall(i = 1:3) transv4f(i)=dot_product(T(:,i),v) ! skip last row
  transv4f(4)=v(4)
end function transv4f


! XXX USES MATRIX SIZE 4X4

subroutine trans44f(T,v) ! 3D trans44form matrix translate T by v
  real, dimension(4,4) :: T
  real, dimension(3)   :: v
  integer              :: i
  forall(i = 1:3) T(4,i)=dot_product(T(1:3,i),v)+T(4,i)
end subroutine trans44f

! XXX END REPLACED ROUTINES

end module transrot

