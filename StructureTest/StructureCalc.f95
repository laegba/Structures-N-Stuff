subroutine structurecalc(d,n,s,x,v,a)
!
!  structurecalc calclates dynamic structure behavior to test methods
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
!  structurecalc M. Salay 20150109
!
!  Eventually intended to test different structure calculation methods,
!  component failure methods, and time advancement methods, with 
!  the possibility of providing an alternate structure calculation for
!  KSP that can be run on a separate processor thereby reducing the load
!  on the primary process.  For the final version the methods can be
!  coded in a different language if necessary.
!
!
!  Just placeholder forces and time advancement to test communication with drawing
!  - some forces and torques representing linear and torsional springs were applied 
!  - I but did not verify that equation set is complete
!  - Perhaps fixed joints with torsional springs be the reference instead
!    
!  Works - but unstable at start if target direction not in line with initial position
!  - Krakeny behavior
!  - Need to check equations sets
!
!  - Only considering gravitational and inertial force
!  - Assuming all masses are 1 kg
!
!  Possible forces
!    - gravity
!    - thrust
!    - torque
!    - air resistance
!    - collision
!
! To do:
!
!  - Forces and other parameters as arguments
!
!
 use transrot

 implicit none
 integer i

! transferred variables
 integer :: d                   ! problem dimension
 integer :: n                   ! number of parts
 integer :: s                   ! array width
 
 real, dimension(s,n) :: x      ! position and rotation
 real, dimension(s,n) :: v      ! linear and angular velocities

! variables that should be transferred
 real,parameter :: dt=1.d-2      ! time step size
 real,parameter :: g=9.80665     ! acceleration of gravity, m/s^2

 real,parameter :: k=500.        ! spring constant
 real,parameter :: b=5.          ! damping constant

 real,parameter :: w=1.          ! block thickness
 real,parameter :: h=w/2         ! half block thickness

 real,parameter :: mp=1.         ! mass, kg
 real,parameter :: Q=mp*w**2./6. ! cube moment of inertia


! working variables
 real, dimension(s,n) :: f       ! forces and torques
 real :: dx,dy,dr                ! displacements x, y, r
! real :: dvx,dvy                 ! difference in velocities x, y
 real :: dw                      ! difference in angular velocities

 real, dimension(s,n) :: m       ! mass/inertia array

 real, dimension(s,n-1) :: dxa   ! differences (joints)

 real,dimension(d,n) :: n1,n2    ! Each part has 2 nodes (for now)
 real,dimension(d,n) :: c        ! Cosines

 real, dimension(s) :: a         ! anchor position
  
  
 integer :: ia=1                 ! anchor index

! XXX all objects have same mass and moment of inertia for now
 m(1:d,1:n)=mp                   ! set mass
 m(d+1:s,1:n)=Q                  ! set moment of inertia

! XXX Using a hardcoded anchor for now - first nodes, 2D: considers x,y position and rotation

! Rotating anchor
 if (d==2) then
   a(1)=0.
   a(2)=0.
 !a(3)=-1.0 !in 3D, this is interpreted as a z translation rather than rotation
! a(3)= x(3,1)+.08
   a(3)= a(3)+dt*1.
 else if (d==3) then
   a(1)= 0.
   a(2)= 0.
   a(3)= 0. 
   a(4)= a(4)+dt*1.
   a(5)= a(5)+dt*1.9
   a(6)= a(6)+dt*2.1
 end if


! dyn indices (parameter,object number) (c++ with reversed order)
!
! Order of x parameters:
!   1. x,y position for 2D and x,y,z for 3D
!   2. rotation r for 2D and rx, ry, rz for 3D


! Order of v parameters:
!   1. x,y velocity for 2D and x,y,z velocity for 3D
!   2. angular velocity w for 2D and wx, wy, wz for 3D
!
!
!
!   Calcualate first as damped springs...
!   Then as truss (shared nodes)
!     -stretchy/bendable truss
!
!
!   Just doing simple test getting forces and torques about right... 
!     Return to do precisely later
!
!
! Interacting structures; 
!     F = −kx − Bv − ma = gravity + thrust = mg + thrust
!
!     root node is connected to anchor
!
!                   v = v + a * dt    ! linear velocity
!                   w = w + n * dt    ! angular velocity
!                   x = x + v * dt    ! displacement
!                   r = r + w * dt    ! rotation
!    where:
!       x-position, v-velocity, a-acceleration, dt-time step
!       r-rotaion, w-angular velocity, n-angular acceleration,

! 
!  Explicit sequential calculation
!
!      - Updates forces, acceleration, velocity, and displacement in that order.
!      - Updated values are used for the subsequent steps.
!
!                   v2 = v1 + a2 * dt    ! linear velocity
!                   w2 = w1 + n2 * dt    ! angular velocity
!                   x2 = x1 + v2 * dt    ! displacement
!                   r2 = r1 + w2 * dt    ! rotation
!
!    where: 1-old time, 2-new time
!
!      - Becomes unstable for high spring and damping constants
!


! Evaluate angular cosines
 if (d == 3) then
   do i=1,n
     c(1:3,i)=dotvf(rotangrf(x(4:6,i)),(/ 0.,1.,0./)) ! y = reference "up" direction 
   end do
 else if (d == 2) then  ! o degrees is straight up
   c(1,1:n)=sin(x(3,1:n))
   c(2,1:n)=cos(x(3,1:n))
 end if

!  Assume  only 2 connection nodes on each object for now (top and bottom)
!  These should be relative positions to facilitate torque calculation

 n1=-h*c   !node 1 relative position
 n2=+h*c   !node 2 relative position


! given angles, calculate cosine 
! 2D:
! x_vector Cos = Cos( theta) = x/h
! y_vector Cos = Sin (theta) = y/h

! initialize forces
 f=0.
 
 f(2,1:n)=f(2,1:n)-m(2,1:n)*g    ! apply gravity force
 dxa(1:d,1:n-1)  = (x(1:d,1:n-1)+n2(1:d,1:n-1)-x(1:d,2:n)-n1(1:d,2:n))*k   ! calculate dx*k
 dxa(d+1:s,1:n-1)= (x(d+1:s,1:n-1)-x(d+1:s,2:n))*k ! calculate rot    dx*k

! bend spring
 f(d+1:s,1:n-1)=f(d+1:s,1:n-1) -dxa(d+1:s,:)            ! bend spring
 f(d+1:s,2:n)  =f(d+1:s,2:n)   +dxa(d+1:s,:)            ! bend spring

! normal force component  ! XXX Should not need to use absolute value here XXX !
 f(1:d,1:n-1)=f(1:d,1:n-1) -  dxa(1:d,1:n-1)*abs(c(1:d,1:n-1)) ! node 2 direction
 f(1:d,2:n)  =f(1:d,2:n)   +  dxa(1:d,1:n-1)*abs(c(1:d,2:n))   ! node 1 direction
! XXX Didn't work without absolute in 2D

! tangengital force component (torque):
 if (d==2) then        !2D
   f(3,1:n-1) =f(3,1:n-1) + n2(1,1:n-1)*dxa(2,1:n-1) - n2(2,1:n-1)*dxa(1,1:n-1)
   f(3,2:n)   =f(3,2:n)   - n1(1,2:n)  *dxa(2,2:n)   + n1(2,2:n)  *dxa(1,2:n)
 else if (d==3) then   !3D
! loop over all joints
   do i=1,n-1
     f(4:6,i)=f(4:6,i) + crossf(-dxa(1:3,i), n2(1:3,i))
     f(4:6,i+1)=f(4:6,i+1) + crossf(dxa(1:3,i), n1(1:3,i+1))
   end do
 end if

! apply damping force (Velocity contribution)
 dxa=v(1:s,1:n-1)-v(1:s,2:n)
 f(1:s,1:n-1)=f(1:s,1:n-1) -b*dxa
 f(1:s,2:n)=f(1:s,2:n)     +b*dxa

! apply boundary conditions: fixed link and/or forces
 ! fixed link to first node
 
 dxa(1:d,ia)=x(1:d,ia)+n1(1:d,ia) 
 dxa(d+1:s,ia)=x(d+1:s,ia)
 dxa(1:s,ia)=dxa(1:s,ia)-a
 f(1:s,ia)=f(1:s,ia)-k*dxa(1:s,ia)
 f(1:s,ia) = f(1:s,ia)-b*v(1:s,ia)

! update velocities and positions
 v = v + dt*f/m
 x = x + dt*v
    
end



