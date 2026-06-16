import {
  requireUser,
  toPublicUser,
  getLicensesForUser,
  getLicenseStats,
} from "../../lib/auth.js";
import { sendJson } from "../../lib/utils.js";

export default async function handler(req, res) {
  if (req.method !== "GET") {
    return sendJson(res, 405, { success: false, message: "Method not allowed" });
  }

  try {
    const user = await requireUser(req, res);
    if (!user) return;

    const licenses = await getLicensesForUser(user.id);
    const stats = await getLicenseStats(user.id);

    sendJson(res, 200, {
      success: true,
      user: toPublicUser(user),
      licenses,
      stats,
    });
  } catch (error) {
    sendJson(res, 500, { success: false, message: error.message || "Failed to load session." });
  }
}
