### For visualizing motion planning algorithms

import tkinter as tk
import customtkinter
import numpy as np

from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure
import time

accel_limit = 15
speed_limit = 8
accel_res = speed_res = 0.1

class GUI():
    def __init__(self):
        customtkinter.set_appearance_mode('system')
        self.root = customtkinter.CTk()
        self.root.title('Motion analysis')
        # self.root.geometry('1800x800+0+0')
        # self.root.minsize(1200, 800)  # Set minimum size for usability
        
        self.GUI_setup()
        
        self.path = []
        self.trajectory_t = []
        self.trajectory_x = []
        self.trajectory_y = []
        self.animation_running = False
        self.last_hash = 0
        
        self.root.after(0, self.update_loop)
        
        self.root.protocol('WM_DELETE_WINDOW', self.on_close)
        self.root.mainloop()
        
    def GUI_setup(self):
        big_font = ('Helvetica', 16)
        small_font = ('Helvetica', 10)
        
        path_frame = customtkinter.CTkFrame(self.root)
        param_frame = customtkinter.CTkFrame(self.root)
        vis_frame = customtkinter.CTkFrame(self.root)
        
        path_frame.grid(row=0, column=0)
        param_frame.grid(row=1, column=0)
        vis_frame.grid(row=0, column=1, rowspan=2)
        
        self.root.grid_columnconfigure(1, weight=1)
        
        # path definition =====================================================
        path_label = tk.Label(path_frame, text='Path', font=big_font)
        path_label.grid(row=0, column=0, columnspan=2, sticky='ew')
        
        self.numcoords = 6
        self.coord_entry = [[None, None] for _ in range(self.numcoords)]
        for i in range(self.numcoords):
            self.coord_entry[i][0] = customtkinter.CTkEntry(path_frame, placeholder_text=f'x{i}', font=small_font)
            self.coord_entry[i][0].grid(row=i+1, column=0)
            self.coord_entry[i][1] = customtkinter.CTkEntry(path_frame, placeholder_text=f'y{i}', font=small_font)
            self.coord_entry[i][1].grid(row=i+1, column=1)
        # default values
        self.coord_entry[0][0].insert(0, '0')
        self.coord_entry[0][1].insert(0, '0')
        self.coord_entry[1][0].insert(0, '1')
        self.coord_entry[1][1].insert(0, '1')
        
        # motion parameters ===================================================
        param_label = tk.Label(param_frame, text='MotionParameters', font=big_font)
        param_label.grid(row=0, column=0, sticky='ew')
        
        self.accel_label = tk.Label(param_frame, font=small_font)
        self.accel_slider = customtkinter.CTkSlider(param_frame,
            from_=accel_res, to=accel_limit, number_of_steps=(int(accel_limit/speed_res)-1))
        self.accel_slider.set(7)
        self.accel_label.grid(row=1, column=0)
        self.accel_slider.grid(row=2, column=0)
        
        self.speed_label = tk.Label(param_frame, font=small_font)
        self.speed_slider = customtkinter.CTkSlider(param_frame,
            from_=speed_res, to=speed_limit, number_of_steps=(int(speed_limit/speed_res)-1))
        self.speed_slider.set(4)
        self.speed_label.grid(row=3, column=0)
        self.speed_slider.grid(row=4, column=0)
        
        # visualization =======================================================
        vis_label = tk.Label(vis_frame, text='Visualization', font=big_font)
        vis_label.grid(row=0, column=0, sticky='ew')
        
        # path preview
        fig = Figure(figsize=(3, 3))
        self.trajectory_ax = fig.add_subplot()
        fig.tight_layout(pad=3)
        
        self.trajectory_ax.set_xticks([])
        self.trajectory_ax.set_yticks([])
        self.trajectory_ax.tick_params(axis='both', which='major', labelsize=12)
        
        self.trajectory_canvas = FigureCanvasTkAgg(fig, master=vis_frame)
        canvas_widget = self.trajectory_canvas.get_tk_widget()
        canvas_widget.grid(row=1, column=0, sticky='nsew', padx=5, pady=5)
        
        animate_button = customtkinter.CTkButton(vis_frame, text='Animate', font=small_font)
        animate_button.bind('<ButtonPress-1>', lambda event: self.animate_loop())
        animate_button.grid(row=2, column=0)
        
        # velocity profile preview
        fig2 = Figure(figsize=(3, 2))
        self.velocity_ax = fig2.add_subplot()
        
        self.velocity_canvas = FigureCanvasTkAgg(fig2, master=vis_frame)
        canvas_widget2 = self.velocity_canvas.get_tk_widget()
        canvas_widget2.grid(row=3, column=0, sticky='nsew', padx=5, pady=5)
        
    def update_loop(self):
        self.update_path()
        self.root.after(100, self.update_loop)
        
    def update_path(self, _=None):
        '''Read inputs for path and motino parameters, calculate the trajectory, and plot it'''
        if self.animation_running: return
        
        # acceleration and max speed
        a = self.accel_slider.get()
        v_max = self.speed_slider.get()
        
        self.accel_label.configure(text=f'acceleration: {a:.4g}')
        self.speed_label.configure(text=f'max. speed: {v_max:.4g}')
        
        # points on the path
        p = []
        for i in range(self.numcoords):
            x = self.coord_entry[i][0].get()
            y = self.coord_entry[i][1].get()
            try:
                if len(p) == 0 or float(x) != p[-1][0] or float(y) != p[-1][1]:
                    p.append((float(x), float(y)))
            except ValueError:
                break
        self.path = tuple(p)
        
        # update the trajectory only when inputs change
        h = hash((self.path, a, v_max))
        if h == self.last_hash: return
        self.last_hash = h
        
        self.calculate_trajectory(self.path, a, v_max, 100)
        self.plot_path()
        self.plot_velocities()
        
        
    def plot_path(self):
        # draw grid
        margin = 1
        x_min = int(np.floor(min(p[0] for p in self.path))) - margin
        x_max = int(np.ceil(max(p[0] for p in self.path))) + margin
        y_min = int(np.floor(min(p[1] for p in self.path))) - margin
        y_max = int(np.ceil(max(p[1] for p in self.path))) + margin
        
        self.trajectory_ax.clear()
        self.trajectory_ax.set_xlim(x_min, x_max)
        self.trajectory_ax.set_ylim(y_min, y_max)
        self.trajectory_ax.set_aspect('equal','box')
        
        for i in range(int(np.floor(x_min)), int(np.ceil(x_max+1))):
            for j in range(int(np.floor(y_min)), int(np.ceil(y_max+1))):
                self.trajectory_ax.plot(i, j, '.k', markersize=3)
                self.trajectory_ax.plot([i-0.5, i-0.5], [y_min, y_max], '-k', linewidth=0.5)
                self.trajectory_ax.plot([x_min, x_max], [j-0.5, j-0.5], '-k', linewidth=0.5)
        
        # plot trajectory
        self.trajectory_ax.plot(self.trajectory_x, self.trajectory_y)
        # plot shortest path
        for i in range(len(self.path) - 1):
            self.trajectory_ax.plot([self.path[i][0], self.path[i+1][0]], [self.path[i][1], self.path[i+1][1]], ':k', linewidth=.5)
        
        self.trajectory_canvas.draw_idle()
        
    def plot_velocities(self):
        # n = len(self.trajectory_t)
        dx = self.trajectory_x[1:] - self.trajectory_x[:-1]
        dy = self.trajectory_y[1:] - self.trajectory_y[:-1]
        dt = self.trajectory_t[1:] - self.trajectory_t[:-1]
        vx = np.zeros(len(dt))
        vy = np.zeros(len(dt))
        for i in range(len(dt)):
            if dt[i] == 0:
                vx[i] = vx[i-1]
                vy[i] = vy[i-1]
            else:
                vx[i] = dx[i] / dt[i]
                vy[i] = dy[i] / dt[i]
        
        self.velocity_ax.clear()
        self.velocity_ax.plot(self.trajectory_t[:-1], vx, 'r')
        self.velocity_ax.plot(self.trajectory_t[:-1], vy, 'b')
        self.velocity_ax.legend(['x', 'y'])
        self.velocity_ax.set_title('Velocity')
        self.velocity_canvas.draw_idle()
        
        
        
    def calculate_trajectory(self, path, a, v_max, numpoints):
        '''
        Calculate a smooth trajectory through the specified points with limits on speed and acceleration
        
        Args:
            path(nx2 array): x, y coordinate pairs
            a(float): maximum acceleration
            v_max(float): maximum speed along a single axis
            numpoints(int): number of points in the trajectory for each segment
        '''
        self.trajectory_t = np.array([])
        self.trajectory_x = np.array([])
        self.trajectory_y = np.array([])
        if len(path) < 2: return
        
        node_vx, node_vy = self.calculate_corner_velocities(path, v_max)
        print(node_vx, node_vy)
        
        for i in range(1, len(path)):
            #start and end positions of the segment
            x0, y0 = path[i-1]
            x1, y1 = path[i]
            dx = x1 - x0
            dy = y1 - y0
            sx = np.sign(dx)
            sy = np.sign(dy)
            
            # start and end velocities
            v0x = node_vx[i-1]
            vfx = node_vx[i]
            v0y = node_vy[i-1]
            vfy = node_vy[i]
            
            # check which axis needs more time to reach its position
            t1x, t2x, t3x = self.get_time_split(sx*dx, a, v_max, sx*v0x, sx*vfx)
            t1y, t2y, t3y = self.get_time_split(sy*dy, a, v_max, sy*v0y, sy*vfy)
            
            # The slower axis uses the shortest-time path. The other follows a cubic spline that finishes at the same time
            if t3x > t3y:
                segment_time = np.linspace(0, t3x, numpoints)
                x = self.get_trajectory_from_times(v0x, sx*a, t1x, t2x, t3x, segment_time)
                y = self.get_cubic_spline(dy, v0y, vfy, t3x, segment_time)
                # update node velocity estimate
                node_vx[i] = float(v0x + sx*a * (t1x - (t3x-t2x)))
            else:
                segment_time = np.linspace(0, t3y, numpoints)
                y = self.get_trajectory_from_times(v0y, sy*a, t1y, t2y, t3y, segment_time)
                x = self.get_cubic_spline(dx, v0x, vfx, t3y, segment_time)
                node_vy[i] = float(v0y + sy*a * (t1y - (t3y-t2y)))
            
            # add the segment to the trajectory
            if len(self.trajectory_t) == 0:
                self.trajectory_t = segment_time
            else: 
                self.trajectory_t = np.append(self.trajectory_t, segment_time + self.trajectory_t[-1])
            self.trajectory_x = np.append(self.trajectory_x, x0 + x)
            self.trajectory_y = np.append(self.trajectory_y, y0 + y)
            
    def calculate_corner_velocities(self, path, v_max):
        '''Calculate velocities at nodes on the path for smooth motion'''
        vx = [0] * len(path)
        vy = [0] * len(path)
        for i in range(1, len(path)-1):
            # find the combination of velocities that make for a straight path on the incoming and outgoing segments and average them
            dx1 = path[i][0] - path[i-1][0]
            dx2 = path[i+1][0] - path[i][0]
            dy1 = path[i][1] - path[i-1][1]
            dy2 = path[i+1][1] - path[i][1]
            
            if abs(dx1) >= abs(dy1):
                vx1 = np.sign(dx1) * v_max
                vy1 = (dy1 / dx1) * vx1
            else:
                vy1 = np.sign(dy1) * v_max
                vx1 = (dx1 / dy1) * vy1
                
            if abs(dx2) >= abs(dy2):
                vx2 = np.sign(dx2) * v_max
                vy2 = (dy2 / dx2) * vx2
            else:
                vy2 = np.sign(dy2) * v_max
                vx2 = (dx2 / dy2) * vy2
                
            vx[i] = float(vx1 + vx2) / 2
            vy[i] = float(vy1 + vy2) / 2
        return vx, vy
            
    def get_time_split(self, dx, a, v_max, v0, vf):
        '''Calculate key times (sop accelerating, start decelerating, finish) to get from x=0 to dx as quickly as possible (dx and a should be positive)'''
        t1 = (v_max - v0) / a
        
        # check if there is time to get to max speed
        dt3 = (v_max - vf) / a
        if v0 * t1 + 0.5 * a * (t1**2 - dt3**2) + v_max * dt3 > dx:
            # if ramping to full speed, then back down would overshoot, we need to stop short of max speed
            t_asym = (vf - v0) / a
            
            # check if there is actually time to achieve the target final velocity
            if vf > v0 and v0 * t_asym + 0.5 * a * t_asym**2 > dx:
                # if not, accelerate from start to finish
                t1 = (-v0 + np.sqrt(v0**2 + 2 * a * dx)) / a
                t2 = t1
                t3 = t1
            elif vf < v0 and v0 * (-t_asym) - 0.5 * a * t_asym**2 > dx:
                t1 = 0
                t2 = 0
                t3 = -(-v0 + np.sqrt(v0**2 - 2 * a * dx)) / a
            else:
            
                # quadratic formula
                aa = a
                bb = -a * t_asym + v0 + vf
                cc = 0.5 * a * t_asym**2 - vf * t_asym - dx
                
                t1 = (-bb + np.sqrt(bb**2 - 4*aa*cc)) / (2 * aa)
                t2 = t1
                t3 = 2 * t1 - t_asym
        
        else:
            dt2 = (dx - v0*t1 - v_max*dt3 + 0.5*a*(dt3**2 - t1**2)) / v_max
            
            t2 = t1 + dt2
            t3 = t2 + dt3
        # 0-t1: acceleration, t1-t2: constant speed, t2-t3: deceleration
        return t1, t2, t3
    
    def get_trajectory_from_times(self, v0, a, t1, t2, t3, time_points):
        '''Calculate a trajectory as follows: start at x=0, v=v0; accelerate at +a until time t1; constant velocity until t2; accelerate at -a until t3'''
        x = np.zeros(len(time_points))
        
        v1 = v0 + t1 * a
        
        x_t1 = 0.5 * a * t1**2 + v0 * t1
        # x_t2 = x_t1 + v_max * (t2 - t1)
        x_t2 = x_t1 + v1 * (t2 - t1)
        for j in range(len(time_points)):
            t = time_points[j]
            if t <= t1:
                x[j] = 0.5 * a * t**2 + v0 * t
            elif t <= t2:
                x[j] = x_t1 + v1 * (t - t1)
            else:
                x[j] = x_t2 + v1 * (t - t2) - 0.5 * a * (t - t2)**2
                
        return x
    
    def get_cubic_spline(self, xf, v0, vf, tf, time_points):
        '''Calculate a cubic trajectory such that: at t=0, x=0, v=v0; at t=tf, x=xf, v=vf'''
        c = v0
        b = (-2*c - vf) / tf + 3*xf / tf**2
        a = (c + vf) / tf**2 - 2*xf / tf**3
        
        return a * time_points**3 + b * time_points**2 + c * time_points
    
    
    def animate_loop(self):
        '''Animate a point following the trajectory in real time'''
        if not self.animation_running:
            # start the loop
            self.plot_path()
            self.animation_start_time = time.time()
            self.animation_running = True
            self.animation_point, = self.trajectory_ax.plot(self.trajectory_x[0], self.trajectory_y[0], 'o')
            
        t = time.time() - self.animation_start_time
        
        if t > self.trajectory_t[-1]:
            self.animation_running = False
            return
        
        i = np.searchsorted(self.trajectory_t, t)
        self.animation_point.set_xdata((self.trajectory_x[i],))
        self.animation_point.set_ydata((self.trajectory_y[i],))
        
        self.root.after(10, self.animate_loop)
        self.trajectory_canvas.draw_idle()
        
        
    def on_close(self):
        self.root.quit()
        self.root.destroy()


if __name__ == '__main__':
    gui = GUI()
