# MAIA Tetris Agent for RL competition
# Bruno Scherrer
#
# v4
# DU controller
#
# v3
# special test to switch on/off forcetetris
#
# v2
# Switch between a risky strategy (forcetetris) and the normal strategy: when the height is below a given threshold (8), only allow to put vertical I on the right
#
# v1
# Optimized weights by Cross Entropy with Dellacherie's features
#
# v0
# This agent implements Dellacherie's algorithm with macros (decisions
# are made when the piece appears, then just applied them until the piece touches the wall)
# A simulator of the deterministic part of the RL simulator
# has been designed by trial and error in order to deal with macro actions efficiently
#
# 


# Display info about the observation/reward/macros/actions + comparison with the internal model
#verbose=True
verbose=False

# Display much more info (macros considered, evaluation, functions entered)
#
#debug=True
debug=False


# Force tetris ? Height under which one only puts vertical I on the right
forcetetris_height=0
forcetetris_height_ref=6

# hack to put forcetetris_height to 0 in specific cases (special test)
special_test=True
special_test_to_be_done=True


# For speed-up
import psyco

# general modules
import sys, copy
from rlglue.agent.Agent import Agent
from rlglue.types import Action
from rlglue.types import Observation






class TetrisAgent(Agent):
	"MAIA Tetris Agent"


########################### Variables

	# Game dimensions
	height=0
	width=0

	# pieces in all their orientations 
	pieces = [ \
		#I
		[ 2, [ [[0,0,0,0], \
			[1,1,1,1], \
			[0,0,0,0], \
			[0,0,0,0]], \
		       [[0,0,1,0], \
			[0,0,1,0], \
			[0,0,1,0], \
			[0,0,1,0]] ] ], \
		#O
		[ 1, [ [[0,0,0,0], \
			[0,1,1,0], \
			[0,1,1,0], \
			[0,0,0,0]] ] ], \
		#T
		[ 4, [ [[0,0,1,0], \
			[0,1,1,0], \
			[0,0,1,0], \
			[0,0,0,0]], \
		       [[0,0,0,0], \
			[0,1,1,1], \
			[0,0,1,0], \
			[0,0,0,0]], \
		       [[0,0,1,0], \
			[0,0,1,1], \
			[0,0,1,0], \
			[0,0,0,0]], \
		       [[0,0,1,0], \
			[0,1,1,1], \
			[0,0,0,0], \
			[0,0,0,0]] ] ], \
		#Z
		[ 2, [ [[0,0,0,0], \
			[0,1,1,0], \
			[0,0,1,1], \
			[0,0,0,0]], \
		       [[0,0,0,0], \
			[0,0,1,0], \
			[0,1,1,0], \
			[0,1,0,0]] ] ], \
		#S
		[ 2, [ [[0,0,0,0], \
			[0,0,1,1], \
			[0,1,1,0], \
			[0,0,0,0]], \
		       [[0,0,0,0], \
			[0,1,0,0], \
			[0,1,1,0], \
			[0,0,1,0]] ] ], \
		#J
		[ 4, [ [[0,0,0,0], \
			[0,1,1,1], \
			[0,0,0,1], \
			[0,0,0,0]], \
		       [[0,0,1,1], \
			[0,0,1,0], \
			[0,0,1,0], \
			[0,0,0,0]], \
		       [[0,1,0,0], \
			[0,1,1,1], \
			[0,0,0,0], \
			[0,0,0,0]], \
		       [[0,0,1,0], \
			[0,0,1,0], \
			[0,1,1,0], \
			[0,0,0,0]] ] ], \
		#L
		[ 4, [ [[0,0,0,1], \
			[0,1,1,1], \
			[0,0,0,0], \
			[0,0,0,0]], \
		       [[0,1,1,0], \
			[0,0,1,0], \
			[0,0,1,0], \
			[0,0,0,0]], \
		       [[0,0,0,0], \
			[0,1,1,1], \
			[0,1,0,0], \
			[0,0,0,0]], \
		       [[0,0,1,0], \
			[0,0,1,0], \
			[0,0,1,1], \
			[0,0,0,0]] ] ] \
		]

	# contains the same information as piece but is optimized: will be initialized automatically by pieces2_init()
	pieces2=[]


	# first last x/y in the above arrays: will be initialized automatically by pieces_xy_init()
	pieces_x=[]
	pieces_y=[]

	# the board + a buffer (for one step ahead evaluation)
	board=[]
	board2=[]

	# current piece
	piecenb=0
	x,y,orient=0,0,0
	piecenb=0

	# list of macros: will be initialized automatically by macros_init()
	macros=[]
	forcetetris_macros=[]

	# current macro
	current_macro=[]
	current_macro_index=0

	# stats: internal count of the number of lines (0,1,2,3,4), associated rewards, heights
	last_number_of_removed_lines=0
	number_of_lines=[]
	rewards_for_lines=[]
	heights=[]

	# gives the index where the first height is found (optimization)
	first_height=0

	# 
	sum_rewards=0
	step_number=0


	# precomputations of range(width) and range(height)
	range_width=[]
	range_height=[]


	# array of size self.height (initialized at start - optimization)
	rows_with_holes_list = []

############################### Methods


	def pieces2_init(self):
		self.pieces2=[]
		for i in range(len(self.pieces)):
			self.pieces2.append([])
			nbor=self.pieces[i][0]
			for j in range(nbor):
				self.pieces2[i].append([])
				p=self.pieces[i][1][j]
				for x in range(4):
					for y in range(4):
						if p[y][x]==1:
							self.pieces2[i][j].append((x,y))		
			
	def pieces_xy_init(self):		
		"Initializes the array pieces_x and pieces_y that contain the dimensions"
		l2x,l2y=[],[]
		for i in range(len(self.pieces)):			
			nbor=self.pieces[i][0]
			lx,ly=[],[]
			for j in range(nbor):				
				xmin,ymin=4,4
				xmax,ymax=-1,-1
				p=self.pieces[i][1][j]		
				for (x,y) in self.pieces2[i][j]:
					if x<xmin:
						xmin=x
					if x>xmax:
						xmax=x
					if y<ymin:
						ymin=y
					if y>ymax:
						ymax=y
				lx.append([xmin,xmax])
				ly.append([ymin,ymax])
			l2x.append(lx)
			l2y.append(ly)
		self.pieces_x=l2x
		self.pieces_y=l2y


	def generate_macro_action(self, piecenb, nbtr, nbrot):
		"generates a list of actions from numbers of translations and rotations"
		# first action is a translation (or nothing)
		if nbtr>0:
			l=[1] # right			
		elif nbtr<0:
			l=[0] # left
		elif nbrot!=0 and (piecenb==0 or piecenb==5):
			l=[4] # nop (when piece is I or J: need to wait before first rotation)
		else:
			l=[] # drop directly
		# following actions are rotations
		if nbrot>0:
			l=l+([2]*nbrot) # rotate left
		elif nbrot<0:
			l=l+([3]*(-nbrot)) # rotate right
		# then the remaining transitions
		if nbtr>0:
			l=l+([1]*(nbtr-1)) # right
		elif nbtr<0:
			l=l+([0]*(-nbtr-1)) # left
		l.append(5) # drop action at the end
		return l




	def macros_init(self):
		"Initializes the array of macros for all pieces"
		self.macros=[]
		self.forcetetris_macros=[]
		for piecenb in range(len(self.pieces)):
			self.macros.append([])
			self.forcetetris_macros.append([])
			x=self.width/2-2                               # always in the middle
			y=-self.pieces_y[self.piecenb][self.orient][0] # always on top
			nbor=self.pieces[piecenb][0]		
			for ori in [[0],[-1,0],[-2,-1,0,1]][nbor/2]:         # 1->[0] 2->[-1,0] 4->[-2,-1,0,1]
				px=self.pieces_x[piecenb][(ori+nbor)%nbor]						
				for tr in range( -(x+px[0]), self.width-(x+px[1]) ):
					self.macros[piecenb].append( self.generate_macro_action(piecenb,tr,ori) )
					if (tr<self.width-(x+px[1])-1) or (piecenb,ori)==(0,1): # for forcetetris: consider the last column only if vertical I 
						self.forcetetris_macros[piecenb].append(True)
					else:
						self.forcetetris_macros[piecenb].append(False)
		
		

	def collision(self,x,y,orient):
		"check whether the current piece at position x,y and orientation orient collides with the current board"
		px=self.pieces_x[self.piecenb][orient]
		py=self.pieces_y[self.piecenb][orient]

		if x+px[0]<0 or x+px[1]>=self.width: # out of the board (left side or right side)
			return 1
		if y+py[0]<0 or y+py[1]>=self.height:# out of the board (ex: illegal rotation on top or touching the ground)
			return 1		
		if y+py[1]>=self.first_height: # no test necessary if the piece is higher than the top of the board
			for (dx,dy) in self.pieces2[self.piecenb][orient]:
				if self.board[x+dx][y+dy]==1:
					return 1	
		return 0


	def put_piece_on_board(self,piecenb,x,y,orient,value):
		"Put the piece piecenb on the current board at position x,y and orientation orient (called when such a piece touches the ground)"
		for (dx,dy) in self.pieces2[piecenb][orient]:
			self.board[x+dx][y+dy]=value


	def try_action(self, x, y, orient, action):
		"Simulates one micro action on the current board for the current piece from x,y,orient - the first returned argument is 1 if the piece is on the ground and 0 else"
		nx,ny,norient=x,y,orient
		 		
		if action==0:   # Left
			nx=x-1			
		elif action==1: # Right
       			nx=x+1
		elif action==2: # Rotate left
			nb_orient=self.pieces[self.piecenb][0]
			norient=(orient+1)%nb_orient
		elif action==3: # Rotate right
			nb_orient=self.pieces[self.piecenb][0]
			norient=(orient+nb_orient-1)%nb_orient		
		elif action==5: # drop
			while(self.collision(x, ny, orient)==0):
				ny=ny+1
			ny=ny-1
		if self.collision(nx, ny, norient)!=0: # if the move is not allowed
			nx,ny,norient=x,y,orient                       # then cancel it
		ny=ny+1 # make it go downward
		if self.collision(nx, ny, norient)!=0:
			return 1,nx,ny-1,norient # reached the ground!
		else:
			return 0,nx,ny,norient   # still room to go down



	def print_board(self, board):
		for i in self.range_height:
			line=""
			for j in self.range_width:
				if board[j][i]==0:
					line += "."
				else:
					line += "X"
			print(line)
	
					
					
	def print_observation(self, observation):
		bug=0
		array=observation.intArray
		pieces=["I","O","T","Z","S","J","L"] 
		print "current piece:",pieces[self.piecenb]
		addcurrentpiece = 1 - self.collision(self.x,self.y,self.orient) # don't add the current piece if it collides with the board (useful for gameover situations)
		if addcurrentpiece==1:
			self.put_piece_on_board(self.piecenb,self.x,self.y,self.orient,-1) # show the current piece
		for i in self.range_height:
			line=""
			line2=""
			for j in self.range_width:
				if array[j+i*self.width]==1:
					line=line+"X"
				else:
					line=line+"."
				if self.board[j][i]==1:
					line2=line2+"X"
				elif self.board[j][i]==-1:
					line2=line2+"X"
				else:				
					line2=line2+"."				
			print line," ",line2
			if line!=line2:            # there should not be any difference between our prediction and the observation !!!
				bug=1
		print "wall height=",self.height-self.first_height
		if addcurrentpiece==1:
			self.put_piece_on_board(self.piecenb,self.x,self.y,self.orient,0) # remove it
		sys.stdout.flush()
		if bug==1:
			print "PROBLEM: model difference !!!"
			sys.exit()
			
				

	def get_next_piece(self, observation):
		"Look at the new piece in the observation vector and put it in our simulator"
		if verbose:
			print "Getting a new piece"
		self.piecenb=list(observation.intArray[-9:-2]).index(1) # piecenb coded in the observation vector
		self.orient=0
		self.x=self.width/2-2                               # always in the middle
		self.y=-self.pieces_y[self.piecenb][self.orient][0] # always on top
		self.need_new_piece=0


	def remove_lines(self,l):
		"Remove the lines contained in an ordered list" 
		for yr in l:
			empty=0
			y=yr
			while(empty==0):  # this loop removes all the lines above y
				empty=1
				for x in self.range_width:
					if y>=1 and self.board[x][y-1]==1:
						self.board[x][y]=1
						empty=0
					else:
						self.board[x][y]=0
				y=y-1											

	def check_lines_for_removal(self,ymin,ymax):
		"Returns an ordered list containing the lines to remove"
		l=[]
		y=ymin
		while y<=ymax:
			full=1
			for x in self.range_width:
				if self.board[x][y]!=1:
					full=0
					break
			if full==1:
				l.append(y)
			y += 1
		return l
		

	def set_action_for_rlglue(self):
		self.action.intArray = [self.current_macro[self.current_macro_index]]
		self.action.doubleArray = []
		self.current_macro_index = self.current_macro_index+1


	def do_action(self,observation):					
				
		# Choose an action
		self.set_action_for_rlglue()

		# Do it (in our simulator) then return it (for their simulator)
		actions=["left", "right", "rotate_left", "rotate_right", "nop", "drop"]
		my_action=self.action.intArray[0]
		if verbose:
			print "Action =",actions[my_action]		
		#print "x=",self.x,"y=",self.y,"orient=",self.orient
		ground,self.x,self.y,self.orient=self.try_action(self.x,self.y,self.orient,my_action)
		if ground==1: # piece has reached the ground

			# update the board
			self.put_piece_on_board(self.piecenb,self.x,self.y,self.orient,1)			
			self.need_new_piece=2			
			py=self.pieces_y[self.piecenb][self.orient]
			l=self.check_lines_for_removal(self.y+py[0],self.y+py[1])
			nb_lines_removed=len(l)
			self.remove_lines(l)
			self.first_height = min(self.first_height,self.y+py[0])+nb_lines_removed
			
			# doing stats on the MDP
			self.number_of_lines[nb_lines_removed] += 1
			self.last_nb_lines_removed=nb_lines_removed  #  for the coming reward (see agent_step)
			self.heights[self.height-self.first_height] += 1						
			
		#print "x=",self.x,"y=",self.y,"orient=",self.orient
		return self.action					



	

	def try_macro_action(self,l):
		"try macro action l from current board and current piece on top (original orient should be 0). Returns the final position (should be on the ground as all macros should finish by drop)"
		orient=0
		x=self.x
		y=self.y		
		for action in l:
			ground,x,y,orient=self.try_action(x, y, orient, action)
			if ground==1:
				break   # touches the ground
		return x,y,orient
			

	def copy_board_to_buffer(self):
		board=self.board
		board2=self.board2
		for yy in self.range_height:
			for xx in self.range_width:
				board2[xx][yy] = board[xx][yy]

	def restore_board_from_buffer(self):
		board=self.board
		board2=self.board2
		for yy in self.range_height:
			for xx in self.range_width:
				board[xx][yy] = board2[xx][yy]

	def evaluate_macro(self, l):
		if debug:
			print "considering macro:",
			actions=["left", "right", "rotate_left", "rotate_right", "nop", "drop"]
			for i in l:
				print actions[i],
			print  
					
		x2,y2,orient2 = self.try_macro_action(l)

		self.put_piece_on_board(self.piecenb,x2,y2,orient2,1)

		p = self.pieces[self.piecenb][1][orient2]
		px = self.pieces_y[self.piecenb][orient2]
		py = self.pieces_y[self.piecenb][orient2]

		# number of cells removed
		r = self.check_lines_for_removal(y2+py[0],y2+py[1])
		nb_cell_removed=0
		for y in r:
			x=px[0]
			while x<=px[1]:
				if p[y-y2][x]==1:
					nb_cell_removed=nb_cell_removed+1
				x += 1
		self.remove_lines(r)

		# number of lines removed
		lines_removed = len(r)

		# new height
		first_height2 = min(self.first_height,y2+py[0])+lines_removed

		if debug:
			self.print_board(self.board)
			print "Height=",self.height-first_height2		

		# landing height
		landing_height = self.height-(y2+(py[1]+py[0])/2.0)

		# eroded piece cells
		eroded_piece_cells = lines_removed * nb_cell_removed

		# row transitions
		row_transitions = 2*first_height2 
		y = first_height2
		while y<self.height:
			if self.board[0][y]==0:
				row_transitions += 1
			x = 1
			while x<self.width:
				if self.board[x][y]!=self.board[x-1][y]:
					row_transitions += 1
				x += 1
			if self.board[self.width-1][y]==0:
				row_transitions += 1
			y += 1

		# column transitions
		column_transitions = 0
		for x in self.range_width:
			y = max(1,first_height2)
			while y<self.height:
				if self.board[x][y]!=self.board[x][y-1]:
					column_transitions += 1
				y += 1
			if self.board[x][self.height-1]==0:
				column_transitions += 1

		# hole number, hole depths and rows with holes				
		hole_number = 0
		for y in self.range_height:
			self.rows_with_holes_list[y] = 0
		rows_with_holes = 0		
		hole_depths = 0		
		for x in self.range_width:
			y = first_height2
			while y<self.height and self.board[x][y]==0: # find the first full cell of column x
				y += 1
			y += 1
			full_cells_above=1
			while y<self.height:		
				if self.board[x][y]==0:    # for each free cell under (found a hole)
					# hole_number
					hole_number += 1
					# rows with holes
					if self.rows_with_holes_list[y]==0:
						self.rows_with_holes_list[y]=1
						rows_with_holes += 1
					# hole depth
					hole_depths += full_cells_above
				else:                      # for each full cell
					full_cells_above += 1
				
				y +=1



		# well sums 
		well_sums=0
		y = first_height2
		while y<self.height: # First column
			if self.board[0][y]==0 and self.board[1][y]==1:
				well_sums += 1
				yy = y+1
				while yy<self.height and self.board[0][yy]==0:
					well_sums += 1
					yy += 1
			y += 1
		x = 1
		while x<self.width-1:
			y = first_height2
			while y<self.height: # Inner columns
				if self.board[x-1][y]==1 and self.board[x][y]==0 and self.board[x+1][y]==1:
					well_sums += 1
					yy = y+1
					while yy<self.height and self.board[x][yy]==0:
						well_sums += 1
						yy += 1
				y +=1
			x += 1
		y = first_height2
		while y<self.height: # Last column
			if self.board[self.width-1][y]==0 and self.board[self.width-2][y]==1:
				well_sums += 1
				yy = y+1
				while yy<self.height and self.board[self.width-1][yy]==0:
					well_sums += 1
					yy += 1
			y += 1

		# Dellacherie's magic formula
		# eval = - landing_height + eroded_piece_cells - column_transitions - row_transitions - 4*hole_number - well_sums
		# Improved Dellacherie
		#eval =  -8.593742*landing_height + 4.982987*eroded_piece_cells - 5.315517*row_transitions - 17.01111*column_transitions - 14.26038*hole_number - 6.035001*well_sums
		# BDU
		eval = -12.63*landing_height + 6.60*eroded_piece_cells - 9.22*row_transitions - 19.77*column_transitions - 13.08*hole_number - 10.49*well_sums -1.61*hole_depths -24.04*rows_with_holes
		if debug:
			print "l=", landing_height, "e=",eroded_piece_cells, "dr=",row_transitions, "dc=", column_transitions,  "h=",  hole_number, "well sums=", well_sums
			print "eval=",eval

			
		# restore the board as it was before we tried the macro
		if lines_removed==0:
			self.put_piece_on_board(self.piecenb,x2,y2,orient2,0)
		else: # it is too complicated so we make a plain copy
			self.restore_board_from_buffer()  

		
		return eval
		

	def plan(self):
	        "Set the best macro action"
		if debug:
			print "Planning the next macro..."

		self.copy_board_to_buffer() # make a copy (used in evaluate_macro to restore the board)
		bestl=[]
		besteval=-999999999.9
		nbor=self.pieces[self.piecenb][0]
		index=0
		for l in self.macros[self.piecenb]:					
			if (self.forcetetris_macros[self.piecenb][index]) or (self.height-self.first_height>forcetetris_height):
				evaluation = self.evaluate_macro(l)
				if evaluation > besteval:
					besteval = evaluation
					bestl=l
			index+=1
		self.current_macro = bestl
		self.current_macro_index = 0
		if verbose:
			print "Chosen macro=",
			actions=["left", "right", "rotate_left", "rotate_right", "nop", "drop"]
			for i in self.current_macro:
				print actions[i],
			print  
			


################################ Functions for interacting with RLGlue ################################

	def agent_init(self,taskSpec):
		global forcetetris_height,special_test_to_be_done
		
		if debug:
			print ">agent_init"
		self.action = Action()
		self.action_types = []
		(version,episodic,states,actions,reward) = taskSpec.split(':')
		if debug:
			print "version=",version,"episodic=",episodic,"states=",states,"actions=",actions,"reward=",reward
		(stateDim,stateTypes,stateRanges) = states.split('_',2)
		if debug:
			print "stateDim=",stateDim,"stateTypes=",stateTypes,"stateRanges=",stateRanges
		stateRanges2=stateRanges.split('_')		
		self.height,self.width=eval(stateRanges2[-2])[0],eval(stateRanges2[-1])[0]
		print "* New Tetris MDP *   height=",self.height,"width=",self.width

		# initializations 
		self.range_width=range(self.width)
		self.range_height=range(self.height)		

		self.pieces2_init()
		self.pieces_xy_init()
		self.macros_init()
		
		self.rewards_for_lines=[0.0]*5
		self.number_of_lines=[0]*5
		self.heights=[0]*(self.height+4)

		self.rows_with_holes_list=[0]*self.height

		forcetetris_height=forcetetris_height_ref
		special_test_to_be_done=special_test
	
	def agent_start(self, observation):						
	
		if debug:
			print ">agent_start"
		#print "New game started"
		
		self.sum_rewards=0
		self.step_number=1
		
		self.board=[]
		self.board2=[]
		self.first_height=self.height      # no bricks at the beginning
		for i in self.range_width:			
			self.board.append([0]*self.height)
			self.board2.append([0]*self.height)
			
		self.get_next_piece(observation) # get the first piece

		self.plan()  # plan the corresponding macro

		return self.do_action(observation) # begin applying the macro

	
	def agent_step(self, reward, observation):

		global forcetetris_height,special_test_to_be_done
		
		if debug:
			print ">agent_step"
		if verbose:	
			print "Immediate reward = ",reward
			
		self.sum_rewards += reward
		self.step_number += 1

		if verbose:
			print "step=",self.step_number, "sum_rewards=",self.sum_rewards
		else:
			if self.step_number % 500==0: # every 500 steps
				sys.stderr.write("%i %f    \r" % (self.step_number,self.sum_rewards))
				sys.stderr.flush()				
		
		if self.need_new_piece==2:    # it takes two transitions to get a new state
			self.need_new_piece=1
			self.rewards_for_lines[self.last_nb_lines_removed] += reward # store stats about rewards
			if self.last_nb_lines_removed==3 and special_test_to_be_done: # the first time one makes 3 lines one does a special test
				special_test_to_be_done=False
				if (reward==3) or ((reward<6 or self.height*self.width<130) and (self.height*self.width<200)): # special test
					forcetetris_height=0
				if verbose:
					print "forcetetris_height set to",forcetetris_height
				
			if verbose:
				print "Waiting for the next piece..."
			#self.print_observation(observation)
			return(self.action)                        # do whatever at this step
		elif self.need_new_piece==1:
			self.get_next_piece(observation)		 # getting new piece
			if verbose:
				self.print_observation(observation)
			self.plan() # plan macro action when new piece
		else:
			if verbose:
				self.print_observation(observation)              # just observe
		return self.do_action(observation)                       # do the action in the macro action
	
	def agent_end(self,reward):
		if debug:
			print ">agent_end"
		
		print "steps=",self.step_number,"rewards=",self.sum_rewards,"    \r",		
		
		sys.stdout.flush()
		
	
	def agent_cleanup(self):
		if debug:
			print ">agent_cleanup"
		print "steps=",self.step_number,"rewards=",self.sum_rewards,"(ended before game over)"

		# showing some stats for the current MDP		
		print "Heights distribution\n",self.heights, "\n[",		
		tot=0
		for i in self.heights:
			tot+=i
		for i in self.heights:
			print float(i)/tot,
		print "]  tot=",tot
		print "Pieces dropped doing lines\n",self.number_of_lines,"\n[",
		tot=0
		for i in self.number_of_lines:
			tot+=i
		for i in self.number_of_lines:
			print float(i)/tot,
		print "]  tot=",tot
		print "Rewards\n",
		for i in range(5):
			if self.number_of_lines[i]!=0:
				print self.rewards_for_lines[i]/self.number_of_lines[i],
			else:
				print "infty",
		print "\nForceTetris=",forcetetris_height,"\n"
			
		
	def agent_freeze(self):
		if debug:
			print ">agent_freeze"
	
	def agent_message(self,inMessage):
		if debug:
			print ">agent_message signal: ",inMessage
		return None
	


# for psyco speed-up
psyco.bind(TetrisAgent.agent_step)
