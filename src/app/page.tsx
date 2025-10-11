'use client'; // Required for components with user interaction (useState, onClick)

import { useState } from 'react';

export default function HomePage() {
    const [robotIp, setRobotIp] = useState('');
    const [status, setStatus] = useState('Enter Robot IP and connect');

    // Function to send commands to the ESP32 API
    const sendCommand = async (command: string) => {
        if (!robotIp) {
            alert('Please enter the robot IP address first!');
            return;
        }
        try {
            const url = `http://${robotIp}/${command}`;
            await fetch(url);
            console.log(`Sent command: ${command}`);
        } catch (error) {
            console.error(`Failed to send command: ${command}`, error);
            setStatus(`Error: Could not connect to ${robotIp}`);
        }
    };

    // Create a handler for a button that sends a command on press and a stop command on release
    const createButtonHandlers = (action: string, stopAction: string) => ({
        onMouseDown: () => sendCommand(action),
        onMouseUp: () => sendCommand(stopAction),
        onMouseLeave: () => sendCommand(stopAction), // Stop if mouse slides off button
        onTouchStart: (e: React.TouchEvent) => {
            e.preventDefault();
            sendCommand(action);
        },
        onTouchEnd: (e: React.TouchEvent) => {
            e.preventDefault();
            sendCommand(stopAction);
        },
    });

    return (
        <main className="flex min-h-screen flex-col items-center justify-center bg-gray-800 text-white p-4">
            <div className="w-full max-w-md text-center">
                <h1 className="text-4xl font-bold text-blue-400 mb-4">Robot Control 🤖</h1>

                {/* IP Input Section */}
                <div className="flex items-center gap-2 mb-8">
                    <input
                        type="text"
                        value={robotIp}
                        onChange={(e) => setRobotIp(e.target.value)}
                        placeholder="e.g., 192.168.1.5"
                        className="w-full p-2 rounded bg-gray-700 border border-gray-600 focus:outline-none focus:ring-2 focus:ring-blue-500"
                    />
                </div>

                {/* Main Controls Grid */}
                <div className="grid grid-cols-3 grid-rows-3 gap-4 justify-items-center mb-8">
                    <div></div>
                    <button {...createButtonHandlers('forward', 'stop-car')} className="control-btn">↑</button>
                    <div></div>
                    <button {...createButtonHandlers('left', 'stop-car')} className="control-btn">←</button>
                    <button onClick={() => ['stop-car', 'stop-arm', 'stop-gripper'].forEach(sendCommand)} className="w-24 h-24 bg-red-600 rounded-full text-2xl font-bold flex items-center justify-center active:bg-red-700">STOP</button>
                    <button {...createButtonHandlers('right', 'stop-car')} className="control-btn">→</button>
                    <div></div>
                    <button {...createButtonHandlers('reverse', 'stop-car')} className="control-btn">↓</button>
                    <div></div>
                </div>

                {/* Arm and Gripper Controls */}
                <div className="grid grid-cols-2 gap-4">
                    <div className="flex flex-col gap-2">
                        <h3 className="text-lg font-semibold">Arm</h3>
                        <button {...createButtonHandlers('arm-up', 'stop-arm')} className="secondary-btn">Arm ↑</button>
                        <button {...createButtonHandlers('arm-down', 'stop-arm')} className="secondary-btn">Arm ↓</button>
                    </div>
                    <div className="flex flex-col gap-2">
                        <h3 className="text-lg font-semibold">Gripper</h3>
                        <button {...createButtonHandlers('gripper-open', 'stop-gripper')} className="secondary-btn">Open</button>
                        <button {...createButtonHandlers('gripper-close', 'stop-gripper')} className="secondary-btn">Close</button>
                    </div>
                </div>
            </div>
        </main>
    );
}