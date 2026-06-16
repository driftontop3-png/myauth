export function applyMask(mask, useLower, useUpper) {
  const lower = "abcdefghijklmnopqrstuvwxyz";
  const upper = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  const digits = "0123456789";

  let charset = digits;
  if (useLower) charset += lower;
  if (useUpper) charset += upper;
  if (!charset) charset = upper + lower + digits;

  return mask.replace(/\*/g, () => charset[Math.floor(Math.random() * charset.length)]);
}

export function formatExpiry(unit, duration) {
  if (unit === "lifetime") return "Lifetime";
  const label = unit.charAt(0).toUpperCase() + unit.slice(1);
  return `${duration} ${label}`;
}

export function parseBody(req) {
  return new Promise((resolve, reject) => {
    let data = "";
    req.on("data", (chunk) => {
      data += chunk;
    });
    req.on("end", () => {
      if (!data) {
        resolve({});
        return;
      }

      const contentType = req.headers["content-type"] || "";
      if (contentType.includes("application/json")) {
        try {
          resolve(JSON.parse(data));
        } catch (error) {
          reject(error);
        }
        return;
      }

      if (contentType.includes("application/x-www-form-urlencoded")) {
        resolve(Object.fromEntries(new URLSearchParams(data)));
        return;
      }

      try {
        resolve(JSON.parse(data));
      } catch {
        resolve(Object.fromEntries(new URLSearchParams(data)));
      }
    });
    req.on("error", reject);
  });
}

export function sendJson(res, status, payload) {
  res.statusCode = status;
  res.setHeader("Content-Type", "application/json");
  res.end(JSON.stringify(payload));
}

export function methodNotAllowed(res, allowed) {
  res.setHeader("Allow", allowed.join(", "));
  sendJson(res, 405, { success: false, message: "Method not allowed" });
}
