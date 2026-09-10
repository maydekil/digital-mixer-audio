import { StrictMode } from "react";
import { createRoot } from "react-dom/client";
import { MixerPage } from "./features/mixer/MixerPage";
import "./styles/tokens.css";
import "./styles/app.css";

createRoot(document.getElementById("root")!).render(
  <StrictMode>
    <MixerPage />
  </StrictMode>
);
