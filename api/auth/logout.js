import { clearSessionCookie } from "../../lib/auth.js";
import { sendJson } from "../../lib/utils.js";

export default async function handler(req, res) {
  if (req.method !== "POST") {
    return sendJson(res, 405, { success: false, message: "Method not allowed" });
  }

  clearSessionCookie(res);
  sendJson(res, 200, { success: true, message: "Logged out." });
}
