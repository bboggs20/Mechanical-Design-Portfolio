% Ben Boggs - boggsb@usc.edu - 10/30/18

clear all; clc; close all;

% inputs
span = 40; % [in] half-span (single panel)
chord = 8.25; % [in]
tc = 0.161; % t/c
weight = 11.5; % max weight plane will fly with in [lb]
loadfactor = 5; % max g's

bank_angle = acos(1/loadfactor)*180/pi % unsuppress output to see max allowable bank angle

fudgefactor = .7333; % testing-based correction 

ult_cap_stress = 80000 * fudgefactor; % psi; 80000 for carbon uni (use weaker of tensile & compressive)
safetyf = 1.5; % factor of safety, 1.5 is std for ADT
plyt = .0075; % single ply thickness in [in].
ds = .01; % numerical increment

x = (0:ds:span);

web = 0.*x + .25; % user defined web thickness function. Spar cap will mimic

avgpanellift = weight*loadfactor/2;
semiarea = span*chord;
shear = avgpanellift-(avgpanellift.*x/span);
moment = shear.*(span-x)./2;

% calculations assume balsa shear web 

Vcap = 0;
Vweb = 0;
prevplies = 0;
cplies = 1;
hingem = "";
for (i=1:length(x))
    mall(i) = moment(i)*safetyf;
    if (x(i) == 18) 
        hingem = ("Allowable moment at hinge: " + string(mall(i)) + " lb*in \n"); % fold point for 2018-19
    end
    I = 2*(((web(i)/12)*plyt^3)+(web(i)*plyt*(((tc*chord)-plyt)/2)^2));
    M = ult_cap_stress*I/(tc*chord);
    j = 1;
    cplies = 1;
    while M < mall % increase plies until the spar doesn't fail
        j = j + 1;
        cplies = cplies + 1;
        b = web(i);
        h = plyt*j;
        d = (tc*chord-h)/2;
        I = 2*(((b*h^3)/12)+b*h*d^2);
        M = ult_cap_stress*I/(tc*chord);
    end
    
    if (cplies ~= prevplies) 
        if (cplies == 1)
            str = ' ply at x = ';
        else
            str = ' plies at x = ';
        end
        fprintf(string(cplies) + str + string(x(i)) + ' in.\n');
    end
    prevplies = cplies;
    
    Vcap = Vcap + ds*j*plyt*web(i)*2;
    Vweb = Vweb + (tc*chord*web(i)*ds)-(ds*j*plyt*web(i)*2);
    cap(i) = j;
    capm(i) = M;
end

figure;
hold on;

fprintf(hingem);

plot(x, moment, 'b', x, mall, 'r--', x, capm, 'k', 'LineWidth', 2);

ax = gca;
ax.XLim = [0, span];
ax.YLim = [0, max(capm)];
xticks((0:5:40));
box off;

capw = Vcap*.0492*2;
webw = Vweb*.0061*2;
sparw = capw + webw;
fprintf('Cap Weight (both sides): ' + string(capw) + ' lb (' + string(capw*16) + ' oz)\n');
fprintf('Web Weight (both sides): ' + string(webw) + ' lb (' + string(webw*16) + ' oz)\n');
fprintf('Spar Weight (both sides): ' + string(sparw) + ' lb (' + string(sparw*16) + ' oz)\n');
fprintf('Spar Weight (one side): ' + string(sparw/2) + ' lb (' + string(sparw*8) + ' oz)\n');

ylabel('Moment [lb*in]');
xlabel('Span Station [in]');
legend('Lifting Moment', 'Allowable Moment', 'Cap Failure Moment');


    