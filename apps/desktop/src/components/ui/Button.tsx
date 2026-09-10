import type { ButtonHTMLAttributes, ReactNode } from "react";

interface ButtonProps extends ButtonHTMLAttributes<HTMLButtonElement> {
  active?: boolean;
  tone?: "cyan" | "amber" | "violet" | "danger" | "green" | "neutral";
  children: ReactNode;
}

export function Button({ active = false, tone = "neutral", className = "", children, ...props }: ButtonProps) {
  return (
    <button className={`ui-button tone-${tone} ${active ? "is-active" : ""} ${className}`} {...props}>
      {children}
    </button>
  );
}
