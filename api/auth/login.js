import {
  getUserByUsername,
  verifyPassword,
  setSessionCookie,
  toPublicUser,
  getLicensesForUser,
  getLicenseStats,
} from "../../lib/auth.js";
import { parseBody, sendJson } from "../../lib/utils.js";

export default async function handler(req, res) {
  if (req.method !== "POST") {
    return sendJson(res, 405, { success: false, message: "Method not allowed" });
  }

  try {
    const body = await parseBody(req);
    const username = String(body.username || "").trim();
    const password = String(body.password || "");

    const user = await getUserByUsername(username);
    if (!user || !(await verifyPassword(password, user.password_hash))) {
      return sendJson(res, 401, { success: false, message: "Invalid username or password." });
    }

    setSessionCookie(res, user.id);

    const licenses = await getLicensesForUser(user.id);
    const stats = await getLicenseStats(user.id);

    sendJson(res, 200, {
      success: true,
      message: "Logged in successfully.",
      user: toPublicUser(user),
      licenses,
      stats,
    });
  } catch (error) {
    sendJson(res, 500, { success: false, message: error.message || "Login failed." });
  }
}
