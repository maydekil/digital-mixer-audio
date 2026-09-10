import type { ReactNode } from "react";

export function Badge({ children, tone = "cyan" }: { children: ReactNode; tone?: "cyan" | "amber" | "violet" | "green" | "red" }) {
  return <span className={`badge tone-${tone}`}>{children}</span>;
}
