clear all; clc;

buffer_size = 60 * 1000;

disp('Connecting to devices..');
motor_controller = SppBluetooth('009ABBE350CC@Exo-Aider', 'demo_motor_controller', buffer_size);
left_sensor_band = []; %SppBluetooth('60FB9912CFA4@Exo-Aider', 'demo_left_sensor_band', buffer_size);
right_sensor_band = SppBluetooth('74E80B12CFA4@Exo-Aider', 'demo_right_sensor_band', buffer_size);
devices = [motor_controller, left_sensor_band, right_sensor_band];

%%
disp('Starting simple tests..');
for device = devices
    name = [device.name, ' (' device.description ')'];

    device.sample_frequency = 1000;
    assert(device.sample_frequency == 1000);
    device.sample_frequency = 500;
    assert(device.sample_frequency == 500);

    device.send_signals_ratio = 10;
    assert(device.send_signals_ratio == 10);
    device.send_signals_ratio = 30;
    assert(device.send_signals_ratio == 30);

    device.sample_frequency = 1000;
    device.send_signals_ratio = 20;
    device.send_signals = true;
    assert(device.send_signals == true);
    device.flush;
    pause(1);
    signals_got = min(device.signal_n);
    assert(45 <= signals_got);
    device.send_signals = false;
    assert(device.send_signals == false);

    disp([' * ', name, ' - pass!']);
end

disp('All simple tests passed!');

%% Make a simulation

disp('Testing real-time simulation performance..');
time_to_run = 20;
sample_frequency = 1000;
send_signals_ratio = 40;
reference_frequency = 4/time_to_run;

% Initial values
n = 0;
t_last = 0;
u_integrator = 0;

% Initialize devices
motor_controller.send('u', [0, 0]);
for device = devices
    disp(['* Configuring "', device.name, '"']);
    device.sample_frequency = sample_frequency;
    device.send_signals_ratio = send_signals_ratio;
    send_frequency = device.send_frequency;
    device.send_signals = true;
end
for device = devices
   device.flush; 
end
disp(['Sampling frequency = ', num2str(sample_frequency), ' Hz']);
disp(['Sending frequency = ' , num2str(send_frequency), ' Hz']);

disp('Simulating...');
sim_start_time = tic;
while toc(sim_start_time) < time_to_run
    n = n + 1;
    
    % Take care of sheduling
    t_now = toc(sim_start_time);
    t_next = t_last + 1 / send_frequency;
    t_wait = t_next - t_now;
    if 0 < t_wait
        pause(t_wait);
        t_last = t_next;
    else
        t_last = toc(sim_start_time); 
        samples_behind = ceil(-t_wait*send_frequency);
        disp(['t = ', num2str(t_now, '%.1f'), 's - Can''t keep up! ', num2str(-t_wait* 1e3, '%.1f') , ' ms behind - ', num2str(samples_behind), ' samples.']);
    end

    % Calculate reference:
    y_n = motor_controller.get_signals('y', 0);
    t_n = t_now; %motor_controller.get_signals('t', -1);
    r_local = calculate_reference(t_n, reference_frequency);

    e = r_local - y_n;
    u_integrator = u_integrator + e / send_frequency;
    u_input = e * 1 + u_integrator * 10;
    u_input = e * 0.5 + u_integrator * 5;
    
    % Simulate the calculation of each sensorband
    for j = 1:2
        t_local = tic; 
        some_counter = 0;
        while  toc(t_local) < 5e-3
            some_counter = some_counter + 1;
        end
    end
    
    % Apply input
    u_input = min([max([u_input, -100]), 100]); % Input saturation
    motor_controller.send('u', [u_input, r_local]);
    
    
    %disp([t_n, u_input, u_n, y_n, r_local]);
end

% Disable devices
motor_controller.send('u', [0, 0]);
for device = devices
    device.send_signals = false;
end
pause(0.1); % wait for remaining messages to arrive.
disp('done');

t_n = motor_controller.get_signals('t');
y_n = motor_controller.get_signals('y');
r_n = motor_controller.get_signals('r');
u_n = motor_controller.get_signals('u');

e_n = r_n-y_n;
r_width = max(r_n)-min(r_n);
percentage_std = 100*std(e_n)/r_width;
percentage_mean = 100*mean(e_n)/r_width;

disp(['mean_error_variation = ', num2str(percentage_mean), '% of max r-variation']);
disp([' std_error_variation = ', num2str(percentage_std), '% of max r-variation']);

figure(1);
subplot(2,1,1);
plot(t_n, y_n, t_n, r_n);
legend('y', 'r');

subplot(2,1,2);
plot(t_n, u_n);
legend('u');

%%
sample_frequency = 1000;
send_signals_ratio = 40;
data_filename = 'my_random_binary_filename.dat';
abs_tol = 0.005;

right_sensor_band.query('reset_sensor_band_counters'); % This is a custom command to reset the internal task counter. If it gets too big, we run into numerical defficulties.
right_sensor_band.sample_frequency = sample_frequency;
right_sensor_band.send_signals_ratio = send_signals_ratio;
right_sensor_band.flush;

disp('Sampling sensor band...');
right_sensor_band.start_writeing_signals_to_file(data_filename, 'test data comment');
right_sensor_band.send_signals = true;
pause(5);
right_sensor_band.send_signals = false;
pause(0.5);
right_sensor_band.stop_writing_signals_to_file();

% Tests:

% hf signal tests:
EMG1 = right_sensor_band.get_signals('EMG1');

EMG1_mod = mod(EMG1, 1);

assert(all(isalmost(EMG1_mod, 0.21, abs_tol))); % All are the same signal type.
assert(all(isalmost(diff(EMG1), 1, abs_tol)));

% lf signal tests:
FSR1 = right_sensor_band.get_signals('FSR1');

FSR1_mod = mod(FSR1, 1);
assert(all(isalmost(FSR1_mod, 0.01, abs_tol)));
FSR1_diff = diff(FSR1(1:send_signals_ratio:end));
assert(all(isalmost(FSR1_diff, send_signals_ratio, abs_tol)));

plot(1:length(EMG1_mod), EMG1_mod, 1:length(FSR1_mod), FSR1_mod);
legend('EMG1', 'FSR1');

% Saved signals:
t = right_sensor_band.load_signals_from_file(data_filename);
delete(data_filename);

assert(all(FSR1 == t.FSR1));
assert(all(EMG1 == t.EMG1));


disp('All tests done.');

%%

function r_n = calculate_reference(t, frequency)
    r_n = 10 * sin(2 * pi * frequency * t);
    r_n(r_n<0) = -5;
end





