import { sql } from "../../lib/db.js";
import { requireUser, toPublicUser, generateSecret } from "../../lib/auth.js";
import { sendJson } from "../../lib/utils.js";

export default async function handler(req, res) {
  if (req.method !== "POST") {
    return sendJson(res, 405, { success: false, message: "Method not allowed" });
  }

  try {
    const user = await requireUser(req, res);
    if (!user) return;

    const appSecret = generateSecret(32);
    const updated = await sql`
      UPDATE users
      SET app_secret = ${appSecret}
      WHERE id = ${user.id}
      RETURNING *
    `;

    sendJson(res, 200, {
      success: true,
      message: "Application secret regenerated.",
      user: toPublicUser(updated.rows[0]),
    });
  } catch (error) {
    sendJson(res, 500, { success: false, message: error.message || "Failed to regenerate secret." });
  }
}
