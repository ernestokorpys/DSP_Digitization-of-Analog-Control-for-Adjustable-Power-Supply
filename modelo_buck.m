close all
clear all
clc

L=150e-6; rL=0.1; C=961e-6; rC=0.13; R=2.2;
f=10000; Vcc=12; Ks=4/5; Kn=1/4;

s=tf('s');
G=minreal(1/((1/(rL+s*L)+1/(R)+s*C/(1+rC*s*C))*(rL+s*L)))

Gp=Vcc*G*Ks*Kn;
step(0.4*Gp)
hold on; step(feedback(Gp,1))
T=1/f;
Gpd=c2d(Gp,T);
step(0.5*Gpd)
Mp=2/100; ts=0.001;