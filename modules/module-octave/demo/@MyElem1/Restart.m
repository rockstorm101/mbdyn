# $Header$
#
# MBDyn (C) is a multibody analysis code. 
# http://www.mbdyn.org
# 
# Copyright (C) 1996-2023
# 
# Pierangelo Masarati	<pierangelo.masarati@polimi.it>
# Paolo Mantegazza	<paolo.mantegazza@polimi.it>
# 
# Dipartimento di Ingegneria Aerospaziale - Politecnico di Milano
# via La Masa, 34 - 20156 Milano, Italy
# http://www.aero.polimi.it
# 
# Changing this copyright notice is forbidden.
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation (version 2 of the License).
# 
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
# 
#################################################################
##
## AUTHOR: Reinhard Resch <mbdyn-user@a1.net>
##        Copyright (C) 2011(-2023) all rights reserved.
##
##        The copyright of this code is transferred
##        to Pierangelo Masarati and Paolo Mantegazza
##        for use in the software MBDyn as described
##        in the GNU Public License version 2.1
##
##################################################################

function Restart(elem, out)
    out.printf("user defined: %d, octave, \"MyElem1\",\n", elem.pMbElem.GetLabel());
    out.printf("node1, %d,\n", elem.pNode1.GetLabel());
    out.printf("offset, %s\n", sprintf("%g, ", elem.o1));
    out.printf("orientation, %s\n", sprintf("%g, ", elem.R1.'));
    out.printf("stiffness x, %g,\n", elem.sx); 
    out.printf("stiffness y, %g,\n", elem.sy);
    out.printf("stiffness z, %g,\n", elem.sz);
    out.printf("damping coefficient, %g;\n", elem.ks);
endfunction
