import { useEffect, useMemo, useState } from "react";
import { Button } from "../../../components/ui/Button";

interface HardwareDevice {
  uid: string;
  name: string;
  defaultInput?: boolean;
  defaultOutput?: boolean;
  inputChannels?: number;
  outputChannels?: number;
  sampleRate?: number;
}

interface EngineResult {
  ok?: boolean;
  error?: string;
  devices?: HardwareDevice[];
  measured?: boolean;
  monitored?: boolean;
  played?: boolean;
  peak?: number;
  inputPeak?: number;
  routeValid?: boolean;
  blackHoleAvailable?: boolean;
  blackHoleUid?: string;
  physicalOutputUid?: string;
  selectedInputStart?: number;
  selectedInputEnd?: number;
  selectedOutputStart?: number;
  selectedOutputEnd?: number;
  blackHoleSampleRate?: number;
  outputSampleRate?: number;
}

interface HardwareMonitorPanelProps {
  open: boolean;
  onClose(): void;
}

export function HardwareMonitorPanel({ open, onClose }: HardwareMonitorPanelProps) {
  const [devices, setDevices] = useState<HardwareDevice[]>([]);
  const [inputUid, setInputUid] = useState("");
  const [outputUid, setOutputUid] = useState("");
  const [status, setStatus] = useState("Native engine required");
  const [routeStatus, setRouteStatus] = useState("System route not checked");
  const [peak, setPeak] = useState(0);
  const [busy, setBusy] = useState(false);

  const inputs = useMemo(() => devices.filter((device) => (device.inputChannels ?? 0) > 0), [devices]);
  const outputs = useMemo(() => devices.filter((device) => (device.outputChannels ?? 0) > 0), [devices]);

  async function send(type: string, payload: Record<string, unknown> = {}) {
    if (!window.localMixer?.engineCommand) {
      setStatus("Open desktop engine mode");
      return null;
    }
    const result = await window.localMixer.engineCommand(type, payload) as EngineResult;
    if (result.ok === false) setStatus(result.error ?? "Engine command failed");
    return result;
  }

  async function refreshDevices() {
    setBusy(true);
    const result = await send("list-devices");
    setBusy(false);
    if (!result?.devices) return;

    setDevices(result.devices);
    const defaultInput = result.devices.find((device) => device.defaultInput && (device.inputChannels ?? 0) > 0) ?? result.devices.find((device) => (device.inputChannels ?? 0) > 0);
    const defaultOutput = result.devices.find((device) => device.defaultOutput && (device.outputChannels ?? 0) > 0) ?? result.devices.find((device) => (device.outputChannels ?? 0) > 0);
    setInputUid((current) => current || defaultInput?.uid || "");
    setOutputUid((current) => current || defaultOutput?.uid || "");
    setStatus(`${result.devices.length} devices`);
  }

  async function meterInput() {
    setBusy(true);
    const result = await send("meter-input", { inputUid, sampleRate: 48000, durationMs: 750 });
    setBusy(false);
    if (!result) return;
    const value = Number(result.peak ?? 0);
    setPeak(value);
    setStatus(result.measured ? `Input peak ${value.toFixed(3)}` : result.error ?? "Input meter failed");
  }

  async function testTone() {
    setBusy(true);
    const result = await send("play-test-tone", { outputUid, sampleRate: 48000, durationMs: 1000, monitorGainDb: -18 });
    setBusy(false);
    setStatus(result?.played ? "Tone played" : result?.error ?? "Tone failed");
  }

  async function monitor() {
    setBusy(true);
    const result = await send("monitor-passthrough", { inputUid, outputUid, sampleRate: 48000, durationMs: 3000, monitorGainDb: -18 });
    setBusy(false);
    const value = Number(result?.inputPeak ?? 0);
    setPeak(value);
    setStatus(result?.monitored ? `Monitor peak ${value.toFixed(3)}` : result?.error ?? "Monitor failed");
  }

  async function checkSystemRoute() {
    setBusy(true);
    const result = await send("routing-system-diagnostics", {
      physicalOutputUid: outputUid,
      sampleRate: 48000,
      blackHoleInputStartChannel: 0,
      physicalOutputStartChannel: 0
    });
    setBusy(false);
    if (!result) return;
    const inputRange = `${Number(result.selectedInputStart ?? 0) + 1}-${Number(result.selectedInputEnd ?? 0) + 1}`;
    const outputRange = `${Number(result.selectedOutputStart ?? 0) + 1}-${Number(result.selectedOutputEnd ?? 0) + 1}`;
    if (result.routeValid) {
      setRouteStatus(`Ready · ${result.blackHoleUid || "BlackHole"} In ${inputRange} -> Out ${outputRange}`);
      setStatus("System route ready");
    } else {
      setRouteStatus(`${result.error ?? "Route rejected"} · BlackHole ${result.blackHoleAvailable ? "found" : "missing"}`);
      setStatus("System route not ready");
    }
  }

  useEffect(() => {
    if (open) void refreshDevices();
  }, [open]);

  if (!open) return null;

  return (
    <div className="modal-backdrop" role="presentation" onMouseDown={onClose}>
      <div className="hardware-monitor-modal" role="dialog" aria-modal="true" aria-label="Hardware monitor" onMouseDown={(event) => event.stopPropagation()}>
        <header>
          <div>
            <strong>HARDWARE MONITOR</strong>
            <span>{status}</span>
          </div>
          <Button onClick={() => void refreshDevices()} disabled={busy}>Refresh</Button>
          <Button tone="danger" onClick={onClose}>Close</Button>
        </header>
        <div className="hardware-monitor-fields">
          <label>
            <span>Input</span>
            <select value={inputUid} onChange={(event) => setInputUid(event.target.value)}>
              <option value="">Default Input</option>
              {inputs.map((device) => <option key={device.uid} value={device.uid}>{device.name || device.uid}</option>)}
            </select>
          </label>
          <label>
            <span>Output</span>
            <select value={outputUid} onChange={(event) => setOutputUid(event.target.value)}>
              <option value="">Default Output</option>
              {outputs.map((device) => <option key={device.uid} value={device.uid}>{device.name || device.uid}</option>)}
            </select>
          </label>
        </div>
        <div className="hardware-monitor-actions">
          <Button tone="cyan" onClick={() => void meterInput()} disabled={busy}>Meter</Button>
          <Button tone="amber" onClick={() => void testTone()} disabled={busy}>Tone</Button>
          <Button tone="green" onClick={() => void monitor()} disabled={busy}>Monitor 3s</Button>
          <Button onClick={() => void checkSystemRoute()} disabled={busy}>Route Check</Button>
          <div className="hardware-peak"><span style={{ width: `${Math.min(100, peak * 100)}%` }} /></div>
        </div>
        <div className="hardware-route-status">
          <span>System</span>
          <strong>{routeStatus}</strong>
        </div>
      </div>
    </div>
  );
}
